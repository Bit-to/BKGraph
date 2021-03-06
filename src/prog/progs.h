#pragma once

#include "widgets/layouts.h"
#include "widgets/graphView.h"

// for handling program state specific things and creating layouts
class ProgState {
public:
	virtual ~ProgState() {}

	virtual void eventKeypress(const SDL_Keysym& key);	// for general keyboard shortcuts
	virtual void eventContextBlank() {}					// gets called when application key is pressed in eventKeypress
	
	virtual Layout* createLayout() = 0;		// for creating the main layout for Scene
	static Popup* createPopupMessage(const string& msg, const vec2<Size>& size);
	static pair<Popup*, LineEdit*> createPopupTextInput(const string& msg, void (Program::*call)(Button*), LineEdit::TextType type, const vec2<Size>& size);
	static Popup* createPopupColorPick(SDL_Color color, Button* clickedBox);
};

class ProgFuncs : public ProgState {
public:
	virtual void eventContextBlank();
	
	virtual Layout* createLayout();
};

class ProgVars : public ProgState {
public:
	virtual void eventContextBlank();

	virtual Layout* createLayout();
};

class ProgGraph : public ProgState {
public:
	virtual void eventKeypress(const SDL_Keysym& key);
	
	virtual Layout* createLayout();
};

class ProgSettings : public ProgState {
public:
	virtual Layout* createLayout();
};
