#include "engine/world.h"

// SIZE

Size::Size(int PIX) :
	usePix(true),
	pix(PIX)
{}

Size::Size(float PRC) :
	usePix(false),
	prc(PRC)
{}

void Size::set(int PIX) {
	usePix = true;
	pix = PIX;
}

void Size::set(float PRC) {
	usePix = false;
	prc = PRC;
}

// WIDGET

Widget::Widget(const Size& SIZ) :
	parent(nullptr),
	id(SIZE_MAX),
	relSize(SIZ)
{}

void Widget::setParent(Layout* PNT, sizt ID) {
	parent = PNT;
	id = ID;
}

void Widget::setRelSize(int pix) {
	relSize.set(pix);
	parent->onResize();
}

void Widget::setRelSize(float prc) {
	relSize.set(prc);
	parent->onResize();
}

vec2i Widget::position() const {
	return parent ? parent->wgtPos(id) : 0;
}

vec2i Widget::size() const {
	return parent ? parent->wgtSize(id) : World::winSys()->resolution();
}

SDL_Rect Widget::rect() const {
	vec2i pos = position();
	vec2i siz = size();
	return {pos.x, pos.y, siz.x, siz.y};
}

// BUTTON

Button::Button(void (Program::*LCL)(Button*), void (Program::*RCL)(Button*), const Size& SIZ) :
	Widget(SIZ),
	lcall(LCL),
	rcall(RCL)
{}

void Button::drawSelf(const SDL_Rect& frame) {
	World::drawSys()->drawRect(overlapRect(rect(), frame), Default::colorNormal);
}

bool Button::onClick(const vec2i& mPos, uint8 mBut) {
	if (mBut == SDL_BUTTON_LEFT && lcall)
		(World::program()->*lcall)(this);
	else if (mBut == SDL_BUTTON_RIGHT && rcall)
		(World::program()->*rcall)(this);
	return true;
}

// CHECK BOX

CheckBox::CheckBox(bool ON, void (Program::*LCL)(Button*), void (Program::*RCL)(Button*), const Size& SIZ) :
	Button(LCL, RCL, SIZ),
	on(ON)
{}

void CheckBox::drawSelf(const SDL_Rect& frame) {
	World::drawSys()->drawRect(overlapRect(rect(), frame), Default::colorNormal);	// draw background
	World::drawSys()->drawRect(overlapRect(boxRect(), frame), on ? Default::colorLight : Default::colorDark);	// draw checkbox
}

bool CheckBox::onClick(const vec2i& mPos, uint8 mBut) {
	if (mBut == SDL_BUTTON_LEFT && lcall) {
		on = !on;
		(World::program()->*lcall)(this);
	} else if (mBut == SDL_BUTTON_RIGHT && rcall)
		(World::program()->*rcall)(this);
	return true;
}

SDL_Rect CheckBox::boxRect() const {
	vec2i pos = position();
	vec2i siz = size();
	int margin = (siz.x > siz.y) ? siz.y/4 : siz.x/4;
	return {pos.x+margin, pos.y+margin, siz.x-margin*2, siz.y-margin*2};
}

// COLOR BOX

ColorBox::ColorBox(SDL_Color CLR, void (Program::*LCL)(Button*), void (Program::*RCL)(Button*), const Size& SIZ) :
	Button(LCL, RCL, SIZ),
	color(CLR)
{}

void ColorBox::drawSelf(const SDL_Rect& frame) {
	World::drawSys()->drawRect(overlapRect(rect(), frame), Default::colorNormal);	// draw background
	World::drawSys()->drawRect(overlapRect(boxRect(), frame), color);	// draw colorbox
}

SDL_Rect ColorBox::boxRect() const {
	vec2i pos = position();
	vec2i siz = size();
	int margin = (siz.x > siz.y) ? siz.y/4 : siz.x/4;
	return {pos.x+margin, pos.y+margin, siz.x-margin*2, siz.y-margin*2};
}

// SLIDER

Slider::Slider(int MIN, int MAX, int VAL, void (Program::*LCL)(Button*), void (Program::*RCL)(Button*), const Size& SIZ) :
	Button(LCL, RCL, SIZ),
	min(MIN),
	max(MAX),
	val(VAL)
{}

void Slider::drawSelf(const SDL_Rect& frame) {
	World::drawSys()->drawSlider(this, frame);
}

bool Slider::onClick(const vec2i& mPos, uint8 mBut) {
	if (mBut == SDL_BUTTON_LEFT) {
		World::scene()->setCapture(this);
		int sx = sliderX();
		if (mPos.x < sx || mPos.x > sx + Default::sliderWidth)	// if mouse outside of slider but inside bar
			setSlider(mPos.x - Default::sliderWidth/2);
		diffSliderMouseX = mPos.x - sliderX();	// get difference between mouse x and slider x
	} else if (mBut == SDL_BUTTON_RIGHT && rcall)
		(World::program()->*rcall)(this);
	return true;
}

void Slider::onDrag(const vec2i& mPos, const vec2i& mMov) {
	setSlider(mPos.x - diffSliderMouseX);
}

void Slider::onUndrag(uint8 mBut) {
	if (mBut == SDL_BUTTON_LEFT)	// if dragging slider stop dragging slider
		World::scene()->setCapture(nullptr);
}

void Slider::setSlider(int xpos) {
	val = (xpos - position().x) * max / sliderL();
	bringIn(val, min, max);

	if (lcall)
		(World::program()->*lcall)(this);
}

void Slider::setMin(int MIN) {
	min = MIN;
	bringIn(val, min, max);
}

void Slider::setMax(int MAX) {
	max = MAX;
	bringIn(val, min, max);
}

void Slider::setVal(int VAL) {
	val = VAL;
	bringIn(val, min, max);
}

int Slider::sliderX() const {
	return position().x + val * sliderL() / max;
}

int Slider::sliderL() const {
	return size().x - Default::sliderWidth;
}

SDL_Rect Slider::barRect() const {
	vec2i pos = position();
	vec2i siz = size();
	return {pos.x, pos.y + siz.y/4, siz.x, siz.y/2};
}

SDL_Rect Slider::sliderRect() const {
	vec2i pos = position();
	vec2i siz = size();
	return {sliderX(), pos.y, Default::sliderWidth, siz.y};
}

// LABEL

Label::Label(const string& TXT, void (Program::*LCL)(Button*), void (Program::*RCL)(Button*), const Size& SIZ, Alignment ALG) :
	Button(LCL, RCL, SIZ),
	align(ALG),
	text(TXT)
{}

void Label::drawSelf(const SDL_Rect& frame) {
	World::drawSys()->drawLabel(this, frame);
}

vec2i Label::textPos() const {
	SDL_Rect rct = rect();
	if (align == Alignment::left)
		return vec2i(rct.x + Default::textOffset, rct.y);
	if (align == Alignment::center)
		return vec2i(rct.x + (rct.w - World::winSys()->getFontSet().textLength(text, rct.h))/2, rct.y);
	return vec2i(rct.x + rct.w - World::winSys()->getFontSet().textLength(text, rct.h) - Default::textOffset, rct.y);	// Alignment::right
}

// LINE EDITOR

LineEdit::LineEdit(const string& TXT, void (Program::*LCL)(Button*), void (Program::*RCL)(Button*), const Size& SIZ, TextType TYP) :
	Label(TXT, LCL, RCL, SIZ, Alignment::left),
	textType(TYP),
	textOfs(0),
	cpos(0)
{
	cleanString(text, textType);
}

bool LineEdit::onClick(const vec2i& mPos, uint8 mBut) {
	if (mBut == SDL_BUTTON_LEFT) {
		World::scene()->setCapture(this);
		setCPos(text.length());
	} else if (mBut == SDL_BUTTON_RIGHT && rcall)
		(World::program()->*rcall)(this);
	return true;
}

void LineEdit::onKeypress(const SDL_Keysym& key) {
	if (key.scancode == SDL_SCANCODE_LEFT) {	// move caret left
		if (key.mod & KMOD_LALT)	// if holding alt skip word
			setCPos(jumpToWordStart(text, cpos));
		else if (key.mod & KMOD_CTRL)	// if holding ctrl move to beginning
			setCPos(0);
		else if (cpos != 0)	// otherwise go left by one
			setCPos(cpos - 1);
	} else if (key.scancode == SDL_SCANCODE_RIGHT) {	// move caret right
		if (key.mod & KMOD_LALT)	// if holding alt skip word
			setCPos(jumpToWordEnd(text, cpos));
		else if (key.mod & KMOD_CTRL)	// if holding ctrl go to end
			setCPos(text.length());
		else if (cpos != text.length())	// otherwise go right by one
			setCPos(cpos + 1);
	} else if (key.scancode == SDL_SCANCODE_BACKSPACE) {	// delete left
		if (key.mod & KMOD_LALT) {	// if holding alt delete left word
			sizt id = jumpToWordStart(text, cpos);
			text.erase(id, cpos - id);
			setCPos(id);
		} else if (key.mod & KMOD_CTRL) {	// if holding ctrl delete line to left
			text.erase(0, id);
			setCPos(0);
		} else if (cpos != 0) {	// otherwise delete left character
			text.erase(cpos - 1);
			setCPos(cpos - 1);
		}
	} else if (key.scancode == SDL_SCANCODE_DELETE) {	// delete right character
		if (key.mod & KMOD_LALT)	// if holding alt delete right word
			text.erase(cpos, jumpToWordEnd(text, cpos) - cpos);
		else if (key.mod & KMOD_CTRL)	// if holding ctrl delete line to right
			text.erase(cpos, text.length() - cpos);
		else if (cpos != text.length())	// otherwise delete right character
			text.erase(cpos, 1);
	} else if (key.scancode == SDL_SCANCODE_HOME)	// move caret to beginning
		setCPos(0);
	else if (key.scancode == SDL_SCANCODE_END)	// move caret to end
		setCPos(text.length());
	else if (key.scancode == SDL_SCANCODE_V) {	// paste text
		if (key.mod & KMOD_CTRL)
			onText(SDL_GetClipboardText());
	} else if (key.scancode == SDL_SCANCODE_C) {	// copy text
		if (key.mod & KMOD_CTRL)
			SDL_SetClipboardText(text.c_str());
	} else if (key.scancode == SDL_SCANCODE_X) {	// cut text
		if (key.mod & KMOD_CTRL) {
			SDL_SetClipboardText(text.c_str());
			setText("");
		}
	} else if (key.scancode == SDL_SCANCODE_Z || key.scancode == SDL_SCANCODE_Y) {	// set text to old text
		if (key.mod & KMOD_CTRL) {
			string newOldCopy = oldText;
			setText(newOldCopy);
		}
	} else if (key.scancode == SDL_SCANCODE_RETURN)
		confirm();
	else if (key.scancode == SDL_SCANCODE_ESCAPE)
		cancel();
}

void LineEdit::onText(string str) {
	cleanString(str, textType);
	text.insert(cpos, str);
	setCPos(cpos + str.length());
}

vec2i LineEdit::textPos() const {
	vec2i pos = position();
	return vec2i(pos.x + textOfs + Default::textOffset, pos.y);
}

void LineEdit::setText(const string& str) {
	oldText = text;
	text = str;
	cleanString(text, textType);
	setCPos(text.length());
}

void LineEdit::setTextType(TextType type) {
	textType = type;
	cleanString(text, textType);
	if (cpos > text.length())
		setCPos(text.length());
}

SDL_Rect LineEdit::caretRect() const {
	vec2i ps = position();
	return {caretPos() + ps.x + Default::textOffset, ps.y, Default::caretWidth, size().y};
}

void LineEdit::setCPos(int cp) {
	cpos = cp;
	checkTextOffset();
}

int LineEdit::caretPos() const {
	return World::winSys()->getFontSet().textLength(text.substr(0, cpos), size().y) + textOfs;
}

void LineEdit::confirm() {
	if (lcall)
		(World::program()->*lcall)(this);

	World::scene()->setCapture(nullptr);
	textOfs = 0;
	oldText = text;
}

void LineEdit::cancel() {
	World::scene()->setCapture(nullptr);
	textOfs = 0;
	setText(oldText);
}

void LineEdit::checkTextOffset() {
	int cp = caretPos();
	int ce = cp + Default::caretWidth;
	int sx = size().x;

	if (cp < 0) {
		textOfs -= cp;
	} else if (ce > sx)
		textOfs -= ce - sx;
}
