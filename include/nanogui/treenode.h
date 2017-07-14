#include <nanogui/nanogui.h>

namespace nanogui
{
        /**
         * \class TreeNode
         *
         * \brief A Widget for displaying tree-like structure.
         *
         * * A TreeNode is a widget with the following layout
         *
         *  \li child 0 - A button that controls the visibility of the
         *                sub-items of the TreeNode.
         *
         *  \li child 1 - the 'display' - a widget that displays
         *                information about this tree node. This is
         *                displayed to the right of the
         *                button. Together the button and the display
         *                make up the 'control-line'.
         *
         *  \li children 2..n - These are the sub-items of the
         *                      TreeNode.  If the button is pressed,
         *                      they are displayed below, and indented
         *                      from, the control-line. If the button
         *                      is not pressed, they are hidden.
         *
         * A TreeNodeLayout has the folowing layout parameters.
         *
         *  \li displaySpacing - the gap between the button and the display.
         *
         *  \li  subItemIndent - The extra space, beyond the width of the
         *                       button, to indent each sub-item of a treenode.
         *
         *  \li controlSpacing - The space between a control line and the
         *                       children of the TreeNode.
         *
         *  \li subItemSpacing - the spacing between subsequent sub-items of a
         *                       TreeNode.
         *
         */
    class NANOGUI_EXPORT TreeNode: public Widget
    {
    public:
        TreeNode(Widget*         parent,
                 int         treeIndent =  8,
                 int      displayIndent =  8,
                 int      subItemIndent = 16,
                 int     controlSpacing =  8,
                 int     subItemSpacing =  4,
                 double connectionWidth =  1.0,
                 bool   drawConnections = true);


        template<typename WidgetClass, typename... Args>
        WidgetClass* setButton(const Args&... args) {
            
            static_assert(std::is_base_of<Button, WidgetClass>::value,
                          "nanogui::TreeNode::setButton() template parameter "
                          "must be derived from nanogui::Button.");
            
            const auto     b =  new WidgetClass(nullptr, args...);
            const auto oldCb = b->changeCallback();
    
            b->setChangeCallback(
                [oldCb, this](bool v)
                {
                    setOpen(v);
                    oldCb(v);
                });
            
            reparentWidget(b, 0);
            recalculateLayout();

            return b;
        }
        

        const Button* button() const{
            return dynamic_cast<const Button*>(childAt(0));
        }

        Button* button() {
            return dynamic_cast<Button*>(childAt(0));
        }

        template<typename WidgetClass, typename... Args>
        WidgetClass* setDisplay(const Args&... args) {
            const auto d =  new WidgetClass(nullptr, args...);
            reparentWidget(d, 1);
            recalculateLayout();
            return d;
        }
        
        const Widget* display() const {
            return childAt(1);
        }

        Widget* display(){
            return childAt(1);
        }

        Widget* subItemAt(int i) {
            return childAt(i+2);
        }

        const Widget* subItemAt(int i) const {
            return childAt(i+2);
        }

        int subItemCount() const {
            return childCount()-2;
        }

        void addSubItem(Widget* w){
            addChild(w);
            recalculateLayout();
        }

        void addSubItem(int i, Widget* w) {
            addChild(i+2, w);
            recalculateLayout();
        }

        void removeSubItem(Widget* w){
            const auto rp = std::remove(mChildren.begin()+2,
                                        mChildren.end(), w);
            if(rp != children().end())
            {
                mChildren.erase(rp,  mChildren.end());
                w->decRef();
                recalculateLayout();
            }
        }

        void removeSubItem(int i) {
            const auto widget = mChildren[i];
            mChildren.erase(mChildren.begin() + i + 2);
            widget->decRef();
            recalculateLayout();
        }

        int treeIndent() const { return mTreeIndent;}
        void setTreeIndent(int treeIndent);

        int displayIndent() const { return mDisplayIndent;}
        void setDisplayIndent(int displayIndent);

        int subItemIndent() const { return mSubItemIndent;}
        void setSubItemIndent(int subItemIndent);

        int controlSpacing() const { return mControlSpacing;}
        void setControlSpacing(int controlSpacing);

        int subItemSpacing() const { return mSubItemSpacing;}
        void setSubItemSpacing(int subItemSpacing);

        double connectionWidth() const { return mConnectionWidth;}
        void setConnectionWidth(double connectionWidth)
        {
            mConnectionWidth = connectionWidth;
        }

        int drawConnections() const { return mDrawConnections;}
        void setDrawConnections(bool drawConnections)
        {
            mDrawConnections = drawConnections;
        }

        Color connectionColor() const { return mConnectionColor;}
        void setConnectionColor(const Color& v) {
            mConnectionColor = v;
        }
        
        bool open() const;
        void setOpen(bool open);

        void draw(NVGcontext *ctx)  override;

            /// Compute the preferred size of the TreeNode, taking into
            /// account whether SubItems are hidden.
        Vector2i preferredSize(NVGcontext *ctx) const override;

            /// Layout the TreeNode, taking not of whether subItems are hidden
        void performLayout(NVGcontext *ctx) override;

        void recalculateLayout();
    private:

        void reparentWidget(Widget* child, int index);

        int    mTreeIndent;
        int    mDisplayIndent;
        int    mSubItemIndent;
        int    mControlSpacing;
        int    mSubItemSpacing;
        double mConnectionWidth;
        bool   mDrawConnections;
        Color  mConnectionColor = Color(255, 255, 255, 255);

    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

        /**
         * A very simple button that is the default button for a TreeNode.
         */
    class TreeButton: public ToolButton
    {
    public:

        TreeButton(Widget*parent=nullptr)
            :ToolButton(parent,ENTYPO_ICON_SQUARED_MINUS)
        {
            setFixedSize(Vector2i(20,20));
            setChangeCallback(
                [this](bool v)
                {
                    setIcon(  v
                            ? ENTYPO_ICON_SQUARED_PLUS
                            : ENTYPO_ICON_SQUARED_MINUS);
                });
        }
    };
}
