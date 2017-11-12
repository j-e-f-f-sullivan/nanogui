/*
    src/scrollpanel.cpp -- Adds vertical & horizontal  scrollbars
    around a widget that is too big to fit into a certain area

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/vscrollpanel.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>

NAMESPACE_BEGIN(nanogui)

ScrollPanel::ScrollPanel(Widget *parent)
    : Widget(parent),
      mChildPreferredWidth(0),
      mChildPreferredHeight(0),
      mScrollX(0.0f),
      mScrollY(0.0f),
      mUpdateLayout(false) { }

void VScrollPanel::performLayout(NVGcontext *ctx) {
    Widget::performLayout(ctx);

    if (mChildren.empty())
        return;
    if (mChildren.size() > 1)
        throw std::runtime_error("VScrollPanel should have one child.");

    Widget *child = mChildren[0];
    mChildPreferredWidth  = child->preferredSize(ctx).x();
    mChildPreferredHeight = child->preferredSize(ctx).y();

    const Vextor2i offset = Vector2i(0,0);
    const Vextor2i   size = mSize;

    if(mChildPreferredWidth > mSize.x()) {

        if(mChildPreferredHeight > mSize.y() -12){
                //scroll x & y
        }
        else
        {
                //scroll x only
            child->setPosition(Vector2i(-mScrollX*(mChildPreferredWidth - mSize.x()), 0));
            child->setSize(Vector2i(mChildPreferredWidth, mSize.y()-12));
        }
        
    }
    else if(mChildPreferredHeight > mSize.y()){
        if(mChildPreferredWidth > (mSize.x() -12)) {
                //scroll x & y
        }
        else
        {
                //scroll y only
            child->setPosition(Vector2i(0, -mScroll*(mChildPreferredHeight - mSize.y())));
            child->setSize(Vector2i(mSize.x()-12, mChildPreferredHeight));
        }
        
    }
    else
    {
            //no scroll
        child->setPosition(Vector2i::Zero());
        child->setSize(mSize);
        mScrollX = 0;
        mScrollY = 0;
    }
    

    child->performLayout(ctx);
}

Vector2i VScrollPanel::preferredSize(NVGcontext *ctx) const {
    if (mChildren.empty())
        return Vector2i::Zero();
    return mChildren[0]->preferredSize(ctx) + Vector2i(12, 0);
}

bool VScrollPanel::mouseDragEvent(const Vector2i &p, const Vector2i &rel,
                            int button, int modifiers) {
    if (!mChildren.empty() && mChildPreferredHeight > mSize.y()) {
        float scrollh = height() *
            std::min(1.0f, height() / (float)mChildPreferredHeight);

        mScroll = std::max((float) 0.0f, std::min((float) 1.0f,
                     mScroll + rel.y() / (float)(mSize.y() - 8 - scrollh)));
        mUpdateLayout = true;
        return true;
    } else {
        return Widget::mouseDragEvent(p, rel, button, modifiers);
    }
}

bool VScrollPanel::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    if (!mChildren.empty() && mChildPreferredHeight > mSize.y()) {
        float scrollAmount = rel.y() * (mSize.y() / 20.0f);
        float scrollh = height() *
            std::min(1.0f, height() / (float)mChildPreferredHeight);

        mScroll = std::max((float) 0.0f, std::min((float) 1.0f,
                mScroll - scrollAmount / (float)(mSize.y() - 8 - scrollh)));
        mUpdateLayout = true;
        return true;
    } else {
        return Widget::scrollEvent(p, rel);
    }
}

void VScrollPanel::draw(NVGcontext *ctx) {
    if (mChildren.empty())
        return;
    Widget *child = mChildren[0];
    child->setPosition(Vector2i(0, -mScroll*(mChildPreferredHeight - mSize.y())));
    mChildPreferredHeight = child->preferredSize(ctx).y();
    float scrollh = height() *
        std::min(1.0f, height() / (float) mChildPreferredHeight);

    if (mUpdateLayout)
        child->performLayout(ctx);

    nvgSave(ctx);
    nvgTranslate(ctx, mPos.x(), mPos.y());
    nvgIntersectScissor(ctx, 0, 0, mSize.x(), mSize.y());
    if (child->visible())
        child->draw(ctx);
    nvgRestore(ctx);

    if (mChildPreferredHeight <= mSize.y())
        return;

    NVGpaint paint = nvgBoxGradient(
        ctx,
        mPos.x() + mSize.x() - 12 + 1,
        mPos.y() + 4 + 1,
        8,
        mSize.y() - 8,

        3, 4, Color(0, 32), Color(0, 92));
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x() + mSize.x() - 12, mPos.y() + 4, 8,
                   mSize.y() - 8, 3);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);

    paint = nvgBoxGradient(
        ctx, mPos.x() + mSize.x() - 12 - 1,
        mPos.y() + 4 + (mSize.y() - 8 - scrollh) * mScroll - 1, 8, scrollh,
        3, 4, Color(220, 100), Color(128, 100));

    nvgBeginPath(ctx);
    nvgRoundedRect(ctx,
                   mPos.x() + mSize.x() - 12 + 1,
                   mPos.y() + 4 + 1 + (mSize.y() - 8 - scrollh) * mScroll,
                   8 - 2,
                   scrollh - 2,
                   2);
    nvgFillPaint(ctx, paint);
    nvgFill(ctx);
}

void VScrollPanel::save(Serializer &s) const {
    Widget::save(s);
    s.set("childPreferredHeight", mChildPreferredHeight);
    s.set("scroll", mScroll);
}

bool VScrollPanel::load(Serializer &s) {
    if (!Widget::load(s)) return false;
    if (!s.get("childPreferredHeight", mChildPreferredHeight)) return false;
    if (!s.get("scroll", mScroll)) return false;
    return true;
}

NAMESPACE_END(nanogui)
