#ifndef _INPUT_H_
#define _INPUT_H_

struct input
{
  int mouseX;
  int mouseY;
  int left;
  int right;
  int middle;
  int mlook;
};
extern struct input input_state;

void keyboardFunction(unsigned char key, int x, int y);
void keyboardSpecialFunction(int key, int x, int y);
void mouseMotionDrag(int x, int y);
void mouseMotion (int x, int y);
void mouseButton(int button, int state, int x, int y);

#endif
