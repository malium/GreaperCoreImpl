# GreaperCoreImpl
 An implementation of the interface GreaperCore.
 
 GreaperCore is a header-only application framework used in the other Greaper projects as a base API.
 This project is an implementation of that framework, this will be used in order to make the API more 
 mature by using a real world example, also other users may find this implementation as a reference for them in order to implement theirs.
 Other Greaper projects may not use this implementation.
 
 In order to build this project you must think how you want to interact with it, as a static library or as a dynamic library. 
 In the case you choose the dynamic library path, you have to remember that in order to initialize the framework you must call the _Greaper function which will return a pointer to the specialization of its GreaperLibrary.
 In the case you choose the static library path you must implement your own GreaperLibrary specialization, which can be an adaptation of the one that is active if you choose the dynamic library path.
 
 Currently there's only one way to build this project, using Visual Studio that supports c++17 targetting win32 in x64 architecture, GreaperCore has parts of it that support Linux distros but it has not been tested yet neither using other IDEs in Windows. 
 