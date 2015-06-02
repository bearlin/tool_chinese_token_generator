● How to verify the xdb file is generated correctly? 
  
  [TC|SC]/output/xdb_non_fuzzy_org.xdb file is created by the application before doing refactor staff.
  
  Please compare [TC|SC]/output/xdb_non_fuzzy.xdb file which is created by new application with 
  [TC|SC]/output/xdb_non_fuzzy_org.xdb file (must be binary same) to ensure that the application 
  does not break anything.

● How to create ALL.txt input file to generate xdb file?
  
  Put China.5000.6613 map and dump tool into map_dump folder or other folder you want.

  To generate ALL.txt file you have to take follow steps:

  1. Dump poi data, retrive POI name, remove empty line and trim

  ./mapdumper  DumpCompressedPoiData --with-rich-poi-addr --map-path ./China.5000.6613 DumpCompressedPoiData.dat
  awk -F', ' '{print $2}' DumpCompressedPoiData.dat > poi_name.txt
  sed 's/^ *//; s/ *$//; /^$/d' poi_name.txt > poi_name_final.txt

  2. Dump addresss, remove first line which is header information, and only retrive city and street name from dump file.

  ./mapdumper DumpCompressedStreetBrowser --map-path ./China.5000.6613 --no-ids CompressedStreetBrowser.dat
  sed '1d'  CompressedStreetBrowser.dat > CompressedStreetBrowser_new.dat
  awk -F ', ' '{print $4 " " $2}' CompressedStreetBrowser_new.dat > CompressedStreetBrowser_final.txt

  3. Combine Address and POI dump files

  cat poi_name_final.txt  CompressedStreetBrowser_final.txt > ALL.txt


  Put the ALL.txt file into XdbFilter/[TC|SC]/input/ folder and modify source code to read corresponding folder to gererate xdb file.


