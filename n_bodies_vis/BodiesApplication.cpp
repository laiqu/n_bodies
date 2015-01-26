#include "BodiesApplication.h"
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <vector>
 
Application::Application(void)
{
}
 
Application::~Application(void)
{
}
 
//-------------------------------------------------------------------------------------
class BodiesFrameListener : public Ogre::FrameListener
{ 
 public:
	BodiesFrameListener(Ogre::SceneManager* mSceneMgr_) : mSceneMgr(mSceneMgr_), sim_time(0), lastNode(0), current(0) {
		std::ifstream in;
		in.open("out");
		int n, dims;
		in >> dims >> n;
		while (in.good()) {
			float time_stamp;
			in >> time_stamp;
			positions.push_back(std::make_pair(time_stamp,
									  std::vector<std::vector<float>>()));		
			for (int i = 0; i < n; i++) {
				std::vector<float> input;	
				for (int j = 0; j < dims + 1; j++) {
					float t;
					in >> t;
					input.push_back(t);
				}
				positions.back().second.push_back(input);
			}
		}
		in.close();		
	}
	bool frameRenderingQueued(const Ogre::FrameEvent& evt) {
	 
		// Create a SceneNode and attach the Entity to it
		sim_time += evt.timeSinceLastFrame;
		std::cout << sim_time << " " << current << std::endl;
		if (sim_time > positions[current].first && current < positions.size()) {
			do {
     			 current++;
			} while (sim_time > positions[current].first && current < positions.size());
			current--;
			if (lastNode)
				lastNode->getCreator()->destroySceneNode(lastNode);
			lastNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(std::to_string(current++));
			for (int i = 0; i < positions[current].second.size(); i++) {
				Ogre::Entity* ogreHead = mSceneMgr->createEntity(std::to_string(10000*current + i), "sphere.mesh");
				auto headNode = lastNode->createChildSceneNode(std::to_string(10001*current + i));
				int mass = positions[current].second[i][0];
				headNode->scale(0.0005f * mass, 0.0005f * mass, 0.0005f * mass);
				// TODO(laiqu) check if we could simply use Ogre::Vector3. To 
				// tired at the moment.
				std::vector<float> tmp({0, 0, 0});
				for (int j = 1; j < positions[current].second[i].size(); j++) {
					tmp[j - 1] = positions[current].second[i][j];
				}	
				headNode->translate(tmp[0], tmp[1], tmp[2]);
				headNode->attachObject(ogreHead);
			}
		}
		return true;
	}
 protected:
	Ogre::SceneManager* mSceneMgr;
 	Ogre::SceneNode* lastNode;
    std::vector<std::pair<float, std::vector<std::vector<float>>>> positions;
    float sim_time;
 	int current;
};
void Application::createScene(void)
{
    // Set the scene's ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
 
    // Create a Light and set its position
    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f);
	mRoot->addFrameListener(new BodiesFrameListener(mSceneMgr));	
}

 
 
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        Application app;
 
        try 
        {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR| MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
#endif
        }
 
        return 0;
    }
 
#ifdef __cplusplus
}
#endif
