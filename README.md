# **UpdateUtility**
#### type: console application 
### Still a work in progress
Driver update/ install for a windows operating system using C++ and the WINAPI. For any newly installed drivers, be sure to add the .INF file into the C:/Windows/System32/DriverStore if it is not already there. This is the default location for driver configurations on window platforms. Compatible with numerous types of Windows OS. Creates a resotre point then updates all the local drivers in the driverstore directory. Creates a log file of any failed install/updates with the purpose of why within the same directory as the application. This file is overwritten with new data every run so be sure to save any wanted information into desired location and name it appropriately.

> Checks to see if system restore is enabled, if not then if enables it.
> Once system restore is enabled, then if creates a restore point to revert to in case of error or driver issue.
> After the restore point has been created, the application will crawl the C:/Windows/System32/DriverStore directory and create a list of all driver configuration files.
> This list of configuration files is then ran through the PNPUTIL.exe tool and reinstalls/updates all of the drivers on the system.
> This application does not download anything. It only installs/ updates local files. This does resolve driver issues.

## Purpose:
> This tool will allow users to fix potential driver issues. Near future plans hope to include the most up to date driver software available for your PC to provide a more comprehesive secrity 
 experience. Read below for detail.

## Future plans:
> Microsoft has their own version of what is considered up to date and is not always actually up to date. And anything greater than this service often requires a monthly subscribtion offered by top AV companies. I would like to integrate either a db or dictionary of driver download url's that can map the .INF files found on a device to the url of the driver download.This will  provide the ability to scan the internet for any new drivers from any vendor or third party drivers to ensure the most up to date drivers are installed on your device. A service that often comes with a subscrition from top-tier AV companies. Also, plans to add switches that would disclude the download of any single/multiple drivers. Feel free to message me if you know of any credible resources or would like to get involved with this project.

<!-- ## Current exe MD5 file hash:
> d71a0c19a3f0c26a38ce2eb4cf3f916f
>
> Please verify file hash for integrity.

Download the most current version <a href="https://github.com/Node0o1/UpdateUtility/blob/main/executable/Windows_Driver_Update_Utility.exe"> **here** </a>. -->

