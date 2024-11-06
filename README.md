# File Scanner
## A Simple File Management App

## Purpose:

This program is to be used to manage a project directory where there may be project folders keps for potentially many years. 
In order to manage the length of time these files stay exposed on the harddrive, this app can be used to determine 
of a project's folder has been ke[pt beyond a **stale date**, and of so, be compressed into an encrypted and compressed 
tar file. Additionally, these files can be deleted should the final type of scan be enabled to identify tar files which
have aged out.

## Configuration:

THis app can be configured either on the command line, or in it's config file. configuration values can be Set
vie key = value pairs. To see a full list of keys and acceptable values, simply run the program at the command 
line with no arguments.