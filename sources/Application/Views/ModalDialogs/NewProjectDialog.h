#ifndef _NEW_PROJECT_DIALOG_H_
#define _NEW_PROJECT_DIALOG_H_

#include "Application/Utils/KeyboardLayout.h"
#include "Application/Views/BaseClasses/ModalView.h"
#include <string>

#define MAX_NAME_LENGTH 12
#define BUTTONS_LENGTH 3

class NewProjectDialog:public ModalView {
public:
  NewProjectDialog(View &view, Path currentPath = "root:");
  virtual ~NewProjectDialog();

  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick);
  virtual void OnFocus();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void CustomizeContextOverlay(const char *&name, const char *&where,
                                       const char *&edit, const char *&field,
                                       const char *&cmd1, const char *&cmd2,
                                       const char *&cmd3, const char *&cmd4,
                                       const char *&cmd5, const char *&cmd6,
                                       const char *&cmd7);

  std::string GetName();

private:
  Path currentPath_;
  int selected_;
  int lastChar_;
  char name_[MAX_NAME_LENGTH + 1];
  int currentChar_;
  bool keyboardMode_;
  int keyboardRow_;
  int keyboardCol_ ;
  void moveCursor(int direction);
};
#endif
