# n_bodies
CUDA N-body simulation.

Simulator part should be easy to compile using provided Makefile (it's possible that CUDA_INSTALL_PATH variable needs some tweaking).
This part is only using CUDA apart from standard libraries.

Visualization part currently is built using:

<code>g++ *.cpp -pthread -I/usr/include/OGRE -I/usr/include/OIS  -I/usr/include/OGRE/Overlay -I/opt/cuda/include -lOgreMain -lpthread -lOIS -lOgreOverlay -lboost_system -std=c++11</code>

If ogre was built manually then this line probably needs different -I flag and additional -L pointing to libs and modify plugins.cfg, so it points to RenderSystemGL lib.
Also we use zip resource, so make sure that you have zzlib installed before compiling ogre (you can also try unzipping SdkTrays.zip and removing Zip path from resources.cfg).
Last thing, we also use OIS (which is usually provided if installing from some repository package, but if compiling from source it also has to be added).
