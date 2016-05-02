#ifndef _APPLICATION_H_
#define _APPLICATION_H_

struct GLFWwindow;

class Application
{
public:
	Application();
	virtual ~Application();

	virtual bool startup();
	virtual void shutdown();

	virtual bool update();
	virtual void draw();
	
    int window_width;
    int window_height;
	GLFWwindow* m_window;
};

#endif //_APPLICATION_H_