#include <nanogui/treenode.h>

using namespace nanogui;



namespace
{
        /*  Find the attatchment point for a widget: the midle of the
         *  left hand side for general widgets, the middle of the left
         *  hand side of the button component of a TreeNode.
         */
    inline Vector2i
    attachmentPoint(Widget* w)
    {
        if(!w)  return Vector2i(0,0);

        const auto   tree = dynamic_cast<TreeNode*>(w);
        const auto    pos = w->position();
        const auto offset = (  (tree && tree->button())
                             ? tree->button()->size().y()
                             : w->size().y())/2;

        return Vector2i(pos.x(), pos.y() + offset);
    }

    inline void
    drawComponent(NVGcontext *ctx, Widget* w)
    {
        if (w->visible()) {
            nvgSave(ctx);
            nvgIntersectScissor(ctx,
                                w->position().x(),
                                w->position().y(),
                                w->width(),
                                w->height());
            w->draw(ctx);
            nvgRestore(ctx);
        }
    }

    inline Vector2i
    preferredComponentSize(NVGcontext *ctx, const Widget* w)
    {
        if(!w)  return Vector2i(0,0);

        const auto ps = w->preferredSize(ctx);
        const auto fs = w->fixedSize();

        return Vector2i (fs[0] ? fs[0] : ps[0],
                         fs[1] ? fs[1] : ps[1]);
    }

    inline Vector2i
    sizeComponent(NVGcontext *ctx, int availableWidth, Widget* w)
    {

        if(!w)  return Vector2i(0,0);
        
        Vector2i ps = Vector2i(availableWidth,
                               w->preferredSize(ctx).y());

        Vector2i fs = w->fixedSize();

        Vector2i targetSize(fs[0] ? fs[0] : ps[0],
                            fs[1] ? fs[1] : ps[1]);

        w->setSize(targetSize);

        return targetSize;
    }

}


TreeNode::TreeNode(Widget*       parent,
                   int       treeIndent,
                   int    displayIndent,
                   int    subItemIndent,
                   int   controlSpacing,
                   int   subItemSpacing,
                   bool drawConnections)
    :Widget(parent),
     mTreeIndent(treeIndent),
     mDisplayIndent(displayIndent),
     mSubItemIndent(subItemIndent),
     mControlSpacing(controlSpacing),
     mSubItemSpacing(subItemSpacing),
     mDrawConnections(drawConnections)
{
        /* Make sure we have slots 0 and 1 available for button and
         * display by adding something innocuous.
         */
    add<TreeButton>();
    add<Label>("undefined");
}

void
TreeNode::reparentWidget(Widget* child, int i)
{
    assert(child);
    assert(!child->parent());
    
    auto current = mChildren[i];

    if(current == child) return;

    if(current)
    {
        current->decRef();
    }

    mChildren[i] = child;
    child->incRef();
    
    child->setParent(this);
    child->setTheme(theme());
  

}

void
TreeNode::setTreeIndent(int treeIndent)
{
    mTreeIndent = treeIndent;
    recalculateLayout();
}

void
TreeNode::setDisplayIndent(int displayIndent)
{
    mDisplayIndent = displayIndent;
    recalculateLayout();
}


void
TreeNode::setSubItemIndent(int subItemIndent)
{
    mSubItemIndent  = subItemIndent;
    recalculateLayout();
}

void
TreeNode::setControlSpacing(int controlSpacing)
{
    mControlSpacing  = controlSpacing;
    recalculateLayout();
}

void
TreeNode::setSubItemSpacing(int subItemSpacing)
{
    mSubItemSpacing  = subItemSpacing;
    recalculateLayout();
}

bool
TreeNode::open() const
{
    const auto b = button();

    return (  b
            ? b->pushed()
            : false);
}

void
TreeNode::setOpen(bool o)
{
    const auto b = button();
    if(b)
    {
        b->setPushed(o);
        recalculateLayout();
    }
}

void
TreeNode::recalculateLayout()
{
     Widget *widget = this;
     while (widget->parent())
         widget = widget->parent();
     ((Screen *) widget)->performLayout();
}

void
TreeNode::draw(NVGcontext *ctx)
{
    nvgSave(ctx);
    nvgTranslate(ctx, position().x(), position().y());

    drawComponent(ctx,  childAt(0));
    drawComponent(ctx, childAt(1));

    const auto          last = children().back();
    const auto       lastPos = attachmentPoint(last);

    const auto          dPos = display()->position();
    const auto         dSize = display()->size();
    const auto      displayY = dPos.y() + dSize.y()/2;
    
    const auto    buttonSize = button()->size();
    const auto decenderStart = Vector2i(dPos.x() - mDisplayIndent, displayY);
    const auto  decenderStop = Vector2i(dPos.x() - mDisplayIndent, lastPos.y());

    const NVGcolor strokeColor = mConnectionColor;
    

    if(mDrawConnections)
    {
        nvgStrokeWidth(ctx, 1.0);
        nvgStrokeColor(ctx, strokeColor);
        
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, buttonSize.x(), decenderStart.y());
        nvgLineTo(ctx, dPos.x(),       decenderStart.y());
        nvgStroke(ctx);
    }
    
    if(open() && (subItemCount() > 0))
    {

        if(mDrawConnections)
        {
            nvgStrokeWidth(ctx, 1.0);
            nvgStrokeColor(ctx, strokeColor);

            nvgBeginPath(ctx);
            nvgMoveTo(ctx, decenderStart.x(), decenderStart.y());
            nvgLineTo(ctx, decenderStop.x(),  decenderStop.y());
            nvgStroke(ctx);
        }

        for(auto i = 0; i < subItemCount(); ++i)
        {
            const auto    item = subItemAt(i);
            const auto itemPos = attachmentPoint(item);

            if(mDrawConnections)
            {
                nvgStrokeWidth(ctx, 1.0);
                nvgStrokeColor(ctx, strokeColor);

                nvgBeginPath(ctx);
                nvgMoveTo(ctx, decenderStart.x(), itemPos.y());
                nvgLineTo(ctx, itemPos.x(),       itemPos.y());
                nvgStroke(ctx);
            }

            drawComponent(ctx, item);
        }
    }

    nvgRestore(ctx);
}



    /// Compute the preferred size of the TreeNode, taking into
    /// account whether SubItems are hidden.
Vector2i
TreeNode::preferredSize(NVGcontext *ctx) const
{
    if(!visible())
    {
        return Vector2i(0,0);
    }

    const auto  buttonSize = preferredComponentSize(ctx,  button());
    const auto displaySize = preferredComponentSize(ctx, display());

    auto height = std::max(buttonSize.y(), displaySize.y());
    auto width  = (  buttonSize.x()
                   + displaySize.x()
                   + mTreeIndent
                   + mDisplayIndent);

    if(open())
    {
        for(auto i = 0; i < subItemCount(); ++i)
        {
            const auto item = subItemAt(i);

            if(!item->visible()) continue;

            if(i == 0) height += mControlSpacing;
            else       height += mSubItemSpacing;

            const auto si = preferredComponentSize(ctx, item);

            height += si.y();
            width   = std::max(width,
                               (  si.x()
                                + buttonSize.x()
                                + mTreeIndent
                                + mSubItemIndent));
        }
    }

    return Vector2i(width, height);
}



    /// Layout the TreeNode, taking not of whether subItems are hidden
void
TreeNode::performLayout(NVGcontext *ctx)
{
    const auto      button = childAt(0);
    const auto     display = childAt(1);

    if(!button || !display) return;
    
    const auto availableWidth = width();
    const auto  buttonSize    = sizeComponent(ctx, availableWidth, button);
    const auto displaySize    = sizeComponent(ctx,
                                              (  availableWidth
                                               - buttonSize.x()
                                               - mTreeIndent
                                               - mDisplayIndent),
                                              display);

    auto            height = std::max(buttonSize.y(),
                                      displaySize.y());

    const auto   midHeight = height/2;

    button->setPosition(Vector2i(0,
                                 midHeight - buttonSize.y()/2));
    button->performLayout(ctx);

    display->setPosition(Vector2i(buttonSize.x() + mTreeIndent + mDisplayIndent,
                                  midHeight - displaySize.y()/2));
    display->performLayout(ctx);

    if(open())
    {
        for(auto i = 0; i < subItemCount(); ++i)
        {
            const auto c = subItemAt(i);

            if(!c->visible()) continue;

            if(i == 0) height += mControlSpacing;
            else       height += mSubItemSpacing;

            const auto si = sizeComponent(ctx,
                                          (  availableWidth
                                           - buttonSize.x()
                                           - mTreeIndent
                                           - mSubItemIndent),
                                          c);

            c->setPosition(Vector2i((  buttonSize.x()
                                     + mTreeIndent
                                     + mSubItemIndent),
                                    height));
            c->performLayout(ctx);

            height += si.y();
        }
    }
}
