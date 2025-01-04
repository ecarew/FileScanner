//
//  main.cpp
//  file scanner
//
//  Created by Evan Carew on 9/19/24.
//
extern "C"{
#include <sqlite3.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
}
#include "config.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
namespace fs = boost::filesystem;
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace boost::posix_time;
using namespace boost::gregorian;

#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <utility>
using namespace std;

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
    return os;
}

void dir_enumerate(vector<fs::path> &p, fs::path &scanpath){
    fs::directory_iterator it(scanpath);
    for (fs::directory_entry& x : it)
        if(fs::is_directory(x)){
            p.push_back(x.path());
        }
}

void store_directories(vector<fs::path>& paths, sqlite3 *db){
    // Iterate over the directories and store them in the db
    string sql(R"END(
    insert into directory (parent, last_altered, name) values(1, 'Vlast_updated', 'Vname');
    )END" );
    const string format = "%Y-%m-%d %H:%M:%S";
    boost::regex name("Vname"), last_updated("Vlast_updated");
    for(auto pth : paths){
        string sql_sub_result = boost::regex_replace(sql, name, (*(--pth.end())).string());
        const std::time_t filetime = fs::last_write_time(pth);
        char buf[70];
        char *zErrMsg = 0;
        int rc = 0;
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&filetime));
        sql_sub_result = boost::regex_replace(sql_sub_result, last_updated, buf);
        rc = sqlite3_exec(db, sql_sub_result.c_str(), NULL, 0, &zErrMsg);
        if(rc!=SQLITE_OK){
            sqlite3_free(zErrMsg);
        }
    }
}

bool test_db(sqlite3 *);
void initdb(sqlite3 *);


//Determine the file's age in days as determined by today's date less the stored date.
int file_age(int id, sqlite3 *db){
    int datedif = 0;
    sqlite3_stmt *stmt;
    string sql(R"END(
               select cast (floor(julianday(datetime()) - julianday((select last_altered from directory where id=?))) as int);
               )END" );
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    datedif = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return datedif;
}

//Get the operations from the database into the vector as pairs of test ops and int durations in days
void load_operations(vector<std::pair<std::string, int>> &operations, sqlite3 *db){
    sqlite3_stmt *stmt;
    string sql(R"END(
            select op, cast(duration as int) from operations;
            )END");
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_step(stmt);
    operations.push_back(make_pair((const char*)sqlite3_column_text(stmt, 0),
                                   sqlite3_column_int(stmt, 1)));
    sqlite3_step(stmt);
    operations.push_back(make_pair((const char*)sqlite3_column_text(stmt, 0),
                                   sqlite3_column_int(stmt, 1)));
    sqlite3_finalize(stmt);
}


class Filetarget{
    string cmd = "tar -czf arch target/. && rm -rf target";
    string enc = "encrypt(openssl enc -aes-256-cbc -salt -in arch -out output -k passwd)";
    string sql = "insert into files values(name, 1)";
public:
    int file_id;
    bool stat = true;
    fs::path name;
    string datetime, pwd;
    Filetarget(int id, string &dt, fs::path &nm, string &pw){
        file_id = id;
        datetime = dt;
        name = nm;
        pwd = pw;
    }
    string doExtendedFileName(){
        //"2023-11-27 21:13:27" Make field 21 chars long
        boost::regex datefix(" ");
        string datestr = boost::regex_replace(datetime, datefix, "_");
        fs::path arch_parent = name.parent_path();
        string fname = name.filename().string();
        arch_parent += datestr + fname + ".tar";
        return arch_parent.string();
    }
    void doTar(){
        if(exists(name)){
            boost::regex arch("arch"), target("target");
            string command = boost::regex_replace(cmd, arch, this->doExtendedFileName());
            //command = boost::regex_replace(cmd,target, name);
            system(command.c_str());
            wait(0);
        } else
            stat = false;
    }
    void doEnc(sqlite3 *db){
        if(exists(name)){
            char *zErrMsg = 0;
            int rc = 0;
            boost::regex arch("arch"), output("output"), passwd("passwd"), name("name");
            string command = boost::regex_replace(enc, arch, doExtendedFileName());
            command = boost::regex_replace(enc, output, doExtendedFileName() + ".enc");
            command = boost::regex_replace(enc, passwd, pwd);
            system(command.c_str());
            std::filesystem::remove(doExtendedFileName());
            string sql = boost::regex_replace(sql, name, doExtendedFileName() + ".enc");
            rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &zErrMsg);
            if(rc!=SQLITE_OK){
                cout << "SQL Error: " << zErrMsg << "\n";
                sqlite3_free(zErrMsg);
                exit(1);
            }
        } else
            stat = false;
    }
};
class FilesToArchive{
    vector<Filetarget> files;
public:
    void doAddFile(Filetarget &ft){
        files.push_back(ft);
    }
    void doArchiveAll(sqlite3 *db){
        for(auto ft : files){
            ft.doTar();
            ft.doEnc(db);
        }
    }
    void doReportAll(){
        cout << "\tFiles in the below list are older than 90 days and have been archived and encrypted\n" << "\tfiles older than 18 months will be removed.\n";
        for(auto ft: files){
            if(ft.stat){
                ptime t(time_from_string(ft.datetime));
                days dd(360 + 180);
                cout << ft.datetime << " | " << ft.name <<  "  Date for removal: " <<
                to_simple_string(t + dd) << "\n";
            }
        }
    }
};
// \fn slightly_stale stale objects, usually directories, aren't ready for removal, however, they are
//ready to be archived. This function directs them to be compressed and renamed such that
//the file name, if discovered, can be used to reconstruct the database records if
//necessary. Format: <Date string with hyphens and plus signs>
// <file name without extension><.gz>
void slightly_stale(vector<std::pair<std::string, int>> &operations, FilesToArchive &stale, string &pw, sqlite3 *db, bool reportOnly=false){
    // first, get the directories and their last updated dates
    string sql(R"END(
            select id, last_altered, name from directory order by id;
            )END");
    sqlite3_stmt *stmt;
    fs::path scan_root;
    int arch_len = 0;
    string arch_op("archive");
    for(std::pair<std::string, int> op: operations)
        if(op.first == arch_op)
            arch_len = op.second;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        int id = sqlite3_column_int(stmt, 0);
        if(id == 1){
            scan_root = (const char*)sqlite3_column_text(stmt, 2);
            continue;
        }
        string last_altered = (const char*)sqlite3_column_text(stmt, 1);
        string name = (const char*)sqlite3_column_text(stmt, 2);
        if(file_age(id, db) > arch_len){
            fs::path arch_dir = scan_root / name;
            name = arch_dir.string();
            Filetarget ft(id, last_altered, arch_dir, pw); //Filetarget(int id, string &dt, fs::path &nm, string &pw)
            stale.doAddFile(ft);
        }
    }
    sqlite3_finalize(stmt);
    if(!reportOnly){
        stale.doArchiveAll(db);
        stale.doReportAll();
    } else {
        stale.doReportAll();
    }
}
//Only to be used with a human in the loop. If agreed, the directory is removed recursively.
void over_ripe(vector<std::pair<std::string, int>> &operations, sqlite3 *db){
    sqlite3_stmt *stmt, *stmt2;
    string sql(R"END(
               select cast (floor(julianday(datetime()) - julianday((select last_altered from files where name=?))) as int);
               )END" );
    string sql_files("select name from files");
    string sql_rm("delete from files where name = '?'");
    string cmd("rm ?");
    boost::regex cfile("?");
    vector<string> names;
    sqlite3_prepare_v2(db, sql_files.c_str(), -1, &stmt, NULL);
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        names.push_back( (const char*)sqlite3_column_text(stmt, 0));
    }
    string arch_op("remove");
    int arch_len = 0;
    for(std::pair<std::string, int> op: operations)
        if(op.first == arch_op)
            arch_len = op.second;
    sqlite3_finalize(stmt);
    sqlite3_prepare_v2(db, sql_files.c_str(), -1, &stmt, NULL);
    for(string name: names){
        sqlite3_bind_text(stmt, 1, name.c_str(), sizeof(name.c_str()), SQLITE_STATIC);
        sqlite3_step(stmt);
        int age = sqlite3_column_int(stmt, 0);
        if(age > arch_len){
            char *zErrMsg = 0;
            string cmd = boost::regex_replace(cmd, cfile, name);
            system(cmd.c_str());
            boost::regex_replace(sql_rm, cfile, name);
            sqlite3_exec(db, sql_rm.c_str(), NULL, 0, &zErrMsg);
        }
    }
}

int main(int argc, const char * argv[]) {
    string config_file, persist_db, short_report, long_report, scanpath, passwd;
    vector<fs::path> paths;
    vector<std::pair<std::string, int>> operations;
    sqlite3 *db;
    FilesToArchive stale;
    try {
        
        // Declare a group of options that will be
        // allowed only on command line
        po::options_description generic("Generic options");
        setupMenuDescriptions(generic, config_file, persist_db, long_report, scanpath);
        
        // Declare a group of options that will be
        // allowed both on command line and in
        // config file
        po::options_description config("Configuration");
        setupMenuDescriptionsC(config, persist_db, short_report, long_report, scanpath, passwd);
        
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config);
        
        po::options_description config_file_options;
        config_file_options.add(config);
        
        po::options_description visible("Allowed options");
        visible.add(generic).add(config);
        
        po::variables_map vm;
        store(po::command_line_parser(argc, argv).
              options(cmdline_options).run(), vm);
        notify(vm);
        fs::path config_path(config_file);
        //fs::canonical(config_file);
        ifstream ifs(config_file.c_str());
        if (!ifs){
            cout << "can not open config file: " << config_file << "\n";
            cout << visible << "\n";
            return 0;
        }else{
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }
        if (vm.count("help") || argc == 1) {
            cout << visible << "\n";
            return 0;
        }
        if (vm.count("version")) {
            cout << "File retention scanner, version 1.0\n";
            return 0;
        }
        if (vm.count("scanpath")){
            cout << "Scan paths are: "
            << vm["scanpath"].as< string >() << "\n";
        }
        if (vm.count("dbfile")){
            cout << "Input persistent file is: "
            << vm["dbfile"].as< string >() << "\n";
        }
        fs::path p(vm["scanpath"].as< string >());
        //First, put the paths in memory. No operations
        dir_enumerate(paths, p);
        //Open the persistent memory database, create if not found
        sqlite3_open(persist_db.c_str(), &db);
        if(vm.count("initdb")){
            cout << "Initializing persistent DB file: " << vm["dbfile"].as< string >() << "\n";
            initdb(db);
        } else if(!test_db(db))
            initdb(db);
        //Store the directory names in the database. Last write time is hashed into the name
        store_directories(paths, db);
        //Get the configured operations from the database.
        load_operations(operations, db);
        //Report on the directories only or do file management operations if asked.
        if (vm.count("sreport") && (vm["sreport"].as< string >() == "y" || vm["sreport"].as< string >() == "Y"
        || vm["sreport"].as< string >() == "t" || vm["sreport"].as< string >() == "T" ) && (!vm.count("archive") && !vm.count("remove"))){
            slightly_stale(operations, stale, passwd, db, true);
        } else {
            slightly_stale(operations, stale, passwd, db);
            if(vm.count("remove")){
                over_ripe(operations, db);
            }
        }
        sqlite3_close(db);
    }
    catch(exception& e)
    {
        cout << e.what() << "\n";
        return 1;
    }
    return 0;
}
