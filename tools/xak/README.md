# Cacao Engine Asset Pack Tool

## About
A tool to work with the Cacao Engine asset pack format (.xak files).

## Capabilities
* Pack creation
* Asset listing with metadata
* Asset extraction
	* Whole pack
	* Individual assets
* Pack merging
* Asset deletion

## Command-Line Usage
```
Cacao Engine Asset Pack Tool 


ce-xak [OPTIONS] SUBCOMMAND


OPTIONS:
  -h,     --help              Print this help message and exit 
  -q,     --quiet             Suppress all output 
  -V,     --verbose           Enable verbose output 
  -v,     --version           Show version info and exit 

SUBCOMMANDS:
  create                      Create a new asset pack 
  list                        List assets in a pack 
  extract                     Extract assets from a pack 
  merge                       Merge two assets packs into a new pack 
  delete                      Delete assets from a pack 
```  
```
Create a new asset pack 


ce-xak create [OPTIONS]


OPTIONS:
  -h,     --help              Print this help message and exit 
  -a,     --assets-dir TEXT:DIR Needs: --res-dir --addr-map -o Excludes: --help-assets-dir 
                              Path to a directory containing assets to place in this pack. 
                              Subdirectories of this path will not be searched. Use 
                              --help-assets-dir to see more info. 
  -r,     --res-dir TEXT:DIR Needs: --assets-dir Excludes: --help-assets-dir 
                              Path to a directory containing arbitray files to embed as 
                              resources in this pack. The folder structure will be copied 
                              as-is. 
  -M,     --addr-map TEXT:FILE Needs: --assets-dir 
                              Path to a file mapping asset filenames to asset addresses for 
                              engine reference 
          --help-assets-dir Excludes: --assets-dir --res-dir -o 
                              View more information about the --assets-dir option 
  -o TEXT REQUIRED Needs: --assets-dir Excludes: --help-assets-dir 
                              Output file path 
```
```
List assets in a pack 


ce-xak list [OPTIONS] input


POSITIONALS:
  input TEXT:FILE REQUIRED    Path to an asset pack file to read as input 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -A,     --assets-only Excludes: --resources-only 
                              Only list assets 
  -R,     --resources-only Excludes: --assets-only --no-meta 
                              Only list resources 
          --no-meta Excludes: --resources-only 
                              Disable printing of asset types 
```
```
Extract assets from a pack 


ce-xak extract [OPTIONS] input [assets...]


POSITIONALS:
  input TEXT:FILE REQUIRED    Path to an asset pack file to read as input 
  assets TEXT ... Excludes: --all-assets
                              Assets to extract from the pack 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -a,     --all-assets Excludes: assets 
                              Extract all assets from the pack 
  -o TEXT REQUIRED            Directory to place output files in 
```
```
Merge multiple assets packs into a single pack 


ce-xak merge [OPTIONS] input...


POSITIONALS:
  input TEXT:FILE ... REQUIRED
                              Asset packs to merge 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -o TEXT REQUIRED            Output file path
```
```
Delete assets from a pack 


ce-xak delete [OPTIONS] input assets...


POSITIONALS:
  input TEXT:FILE REQUIRED    Path to an asset pack file to read as input 
  assets TEXT ... REQUIRED    Assets to delete from the pack 

OPTIONS:
  -h,     --help              Print this help message and exit 
```