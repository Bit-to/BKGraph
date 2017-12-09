#include "world.h"

Scene::Scene() :
	capture(nullptr)
{}

void Scene::onKeypress(const SDL_KeyboardEvent& key) {
	if (LineEdit* le = dynamic_cast<LineEdit*>(capture))
		le->onKeypress(key.keysym);
	else if (!key.repeat)
		World::program()->getState()->eventKeypress(key.keysym);
}

void Scene::onMouseMove(const vec2i& mPos, const vec2i& mMov) {
	updateFocused(mPos);

	if (capture)
		capture->onDrag(mPos, mMov);
}

void Scene::onMouseDown(const vec2i& mPos, uint8 mBut) {
	if (LineEdit* le = dynamic_cast<LineEdit*>(capture))	// mouse button cancels keyboard capture
		le->confirm();
	if (context && context->onClick(mPos, mBut))	// if context menu got clicked
		return;
	if (!focused.back()->onClick(mPos, mBut) && mBut == SDL_BUTTON_RIGHT)	// if nothing got clicked and it's a right click do a blank right click
		World::program()->getState()->eventContextBlank();
}

void Scene::onMouseUp(uint8 mBut) {
	if (capture)
		capture->onUndrag(mBut);
}

void Scene::onMouseWheel(int wMov) {
	if (ScrollArea* box = getFocusedScrollArea())
		box->scrollList(wMov * Default::scrollFactorWheel);
}

void Scene::onText(const string& text) {
	static_cast<LineEdit*>(capture)->onText(text);	// text input should only run if line edit is being captured, therefore a cast check shouldn't be necessary
}

void Scene::onResize() {
	layout->onResize();
	if (popup)
		popup->onResize();
}

void Scene::setLayout(Layout* newLayout) {
	// clear scene
	World::winSys()->getFontSet().clear();
	setCapture(nullptr);
	popup.reset();
	context.reset();

	// set stuff up
	layout.reset(newLayout);
	setFocused(World::winSys()->mousePos());
}

void Scene::setPopup(Popup* newPopup) {
	popup.reset(newPopup);
	setFocused(World::winSys()->mousePos());
}

void Scene::setContext(Context* newContext) {
	context.reset(newContext);
	if (context) {	// if new context correct it's position if it's out of frame
		vec2i res = World::winSys()->resolution();
		correctContextPos(context->position.x, context->getSize().x, res.x);
		correctContextPos(context->position.y, context->height(), res.y);
	}
}

void Scene::correctContextPos(int& pos, int size, int res) {
	if (pos + size > res)
		pos -= size;
	if (pos < 0)
		pos = 0;
}

void Scene::setCapture(Widget* cbox) {
	capture = cbox;
	if (dynamic_cast<LineEdit*>(capture))
		SDL_StartTextInput();
	else
		SDL_StopTextInput();
}

void Scene::setFocused(const vec2i& mPos) {
	Layout* lay = popup.get() ? static_cast<Layout*>(popup.get()) : layout.get();
	focused.resize(1);
	focused[0] = lay;
	setFocusedElement(mPos, lay);
}

void Scene::updateFocused(const vec2i& mPos) {
	// figure out how far in focused needs to be modified
	sizt i = 1;	// no need to check top widget
	for (; i<focused.size(); i++)
		if (!inRect(mPos, overlapRect(focused[i]->rect(), focused[i]->parentFrame())))
			break;

	// get rid of widgets not overlapping with mouse position
	if (i < focused.size())
		focused.erase(focused.begin() + i, focused.end());
	
	// append new widgets if possible
	if (Layout* lay = dynamic_cast<Layout*>(focused.back()))
		setFocusedElement(mPos, lay);
}

void Scene::setFocusedElement(const vec2i& mPos, Layout* box) {
	SDL_Rect frame = box->frame();
	for (Widget* it : box->getWidgets())
		if (inRect(mPos, overlapRect(it->rect(), frame))) {
			focused.push_back(it);
			if (Layout* lay = dynamic_cast<Layout*>(it))
				setFocusedElement(mPos, lay);
			break;
		}
}

ScrollArea* Scene::getFocusedScrollArea() const {
	for (Widget* it : focused)
		if (ScrollArea* sa = dynamic_cast<ScrollArea*>(it))
			return sa;
	return nullptr;
}
