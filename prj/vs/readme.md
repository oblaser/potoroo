# build with Visual Studio

VS does save some project settings in the `.vs` dir. Thus you may want/need to set the following Configuration Properties:

- for all configurations:
   - Debugging > Working Directory: $(TargetDir)
