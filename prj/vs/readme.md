# build with Visual Studio

It seems that VS does save some project settings in the `.vs` dir. Thus you may want/need to set the following Configuration Properties for all Configurations:

- Debugging > Working Directory: $(TargetDir)
