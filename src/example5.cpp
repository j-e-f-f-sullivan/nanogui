#include <nanogui/treenode.h>

using namespace nanogui;

class TreeControlPanel: public Widget
{
public:

    TreeControlPanel(Widget* parent)
        :Widget(parent) {

        setLayout(new GroupLayout());

        new Label(this, "Global Control", "sans-bold", 20);

        AddOpenCloseAll();

        new Label(this, "Tree Layout Parameters", "sans-bold", 20);

        AddTreeIndentSliders();

        AddDrawConnections();

        AddConnectionColorControl();

        new Label(this, "Button Parameters", "sans-bold", 20);
        AddButtonFontColorControl();

        AddButtonSizeSliders();
    }
    
private: 

    void
    AddOpenCloseAll(){
        const auto b = new Button(this, "Open/Close all TreeNodes");
        b->setFontSize(18);
        b->setFlags(Button::ToggleButton);
        b->setChangeCallback(
            [this](bool v){
                OpenCloseAll(window(), v);
            });
    }
    
    void
    OpenCloseAll(Widget * w, bool v) {
        if(!w) return;
        
        const auto t = dynamic_cast<TreeNode*>(w);
        
        if(t){
            t->setExpanded(v);
        }

        for( const auto c: w->children()){
            OpenCloseAll(c,v);
        }
    }

    void
    AddTreeIndentSliders(){
        const auto grid = new Widget(this);

        grid->setLayout(new GridLayout(Orientation::Horizontal,
                                       3,
                                       Alignment::Maximum,
                                       0,
                                       15));

        AddSlider(grid,"treeIndent",mTreeIndent, 20,
                  [this](int v){
                mTreeIndent = v;
                AdjustTree(window());
            });
        
        AddSlider(grid, "displayIndent", mDisplayIndent, 20, [this](int v){
                mDisplayIndent = v;
                AdjustTree(window());
            });

        AddSlider(grid, "subItemIndent", mSubItemIndent, 20, [this](int v)
            {
                mSubItemIndent = v;
                AdjustTree(window());
            });

        AddSlider(grid, "controlSpacing", mSubItemIndent, 20, [this](int v)
            {
                mControlSpacing = v;
                AdjustTree(window());
            });

        AddSlider(grid, "subItemSpacing", mSubItemIndent, 20, [this](int v)
            {
                mSubItemSpacing = v;
                AdjustTree(window());
            });

        AddSlider(grid, "connectionWidth", mConnectionWidth, 3.0, [this](double v)
            {
                mConnectionWidth = v;
                AdjustTree(window());
            });
    }

    void
    AddSlider(Widget*             parent,
              std::string          label,
              int                  value,
              int               maxValue,
              std::function<void(int)> f){
        new Label(parent, label);

        Slider *slider = new Slider(parent);
        slider->setValue((double)value/maxValue);
        slider->setFixedWidth(80);

        TextBox *textBox = new TextBox(parent);
        textBox->setFixedSize(Vector2i(60, 25));
        textBox->setValue(std::to_string(value));
        textBox->setFontSize(20);
        textBox->setAlignment(TextBox::Alignment::Right);
        
        slider->setCallback(
            [f, textBox, maxValue](float value) {
                textBox->setValue(std::to_string((int) (value * maxValue)));
                f((int) (value * maxValue));
            });
    }

    void
    AddSlider(Widget*             parent,
              std::string          label,
              double               value,
              double            maxValue,
              std::function<void(double)> f){
        new Label(parent, label);

        Slider *slider = new Slider(parent);
        slider->setValue(value/maxValue);
        slider->setFixedWidth(80);

        TextBox *textBox = new TextBox(parent);
        textBox->setFixedSize(Vector2i(60, 25));
        textBox->setValue(std::to_string(value));
        textBox->setFontSize(20);
        textBox->setAlignment(TextBox::Alignment::Right);
        
        slider->setCallback(
            [f, textBox, maxValue](float value) {
                auto r = (int)(value * maxValue * 100);
                textBox->setValue(std::to_string(r));
                f((value * maxValue));
            });
    }

    void
    AddDrawConnections(){
        const auto b = new Button(this, "Draw Connections");
        b->setFontSize(18);
        b->setFlags(Button::ToggleButton);
        b->setPushed(true);
        b->setChangeCallback(
            [this](bool v){
                mDrawConnections = v;
                AdjustTree(window());
            });
    }

    void
    AddConnectionColorControl(){
        const auto connectionColor
            = new PopupButton(this, "Connection Color", 0);
        connectionColor->setFontSize(18);
        AddColorSelector(
            connectionColor,
            [this](const Color& v){
                mConnectionColor = v;
                AdjustTree(window());
            });
    }

    void
    AddButtonFontColorControl(){
        const auto connectionColor
            = new PopupButton(this, "Button Font Color", 0);
        connectionColor->setFontSize(18);
        AddColorSelector(
            connectionColor,
            [this](const Color& v){
                mButtonFontColor = v;
                AdjustTree(window());
            });
    }

    void
    AddColorSelector(PopupButton* b,std::function<void(const Color& v)> f){
        const auto popup = b->popup();

        popup->setLayout(new GroupLayout());
        ColorWheel *colorwheel = new ColorWheel(popup);
        colorwheel->setColor(mConnectionColor);


        colorwheel->setCallback(f);
    }
        
    void
    AddButtonSizeSliders(){
        const auto grid = new Widget(this);

        grid->setLayout(new GridLayout(Orientation::Horizontal,
                                       3,
                                       Alignment::Maximum,
                                       0,
                                       15));

        AddSlider(grid, "Button Size", mButtonSize, 50, [this](int v)
            {
                mButtonSize = v;
                AdjustTree(window());
            });
        AddSlider(grid, "Button Font Size", mButtonFontSize, 72, [this](int v)
            {
                mButtonFontSize = v;
                AdjustTree(window());
            });
        new Widget(grid);
    }

    void
    AdjustTree(Widget * w) {
        if(!w) return;

        const auto t = dynamic_cast<TreeNode*>(w);

        if(t)
        {
            t->setConnectionColor(mConnectionColor);

            t->button()->setTextColor(mButtonFontColor);
            t->button()->setFontSize(mButtonFontSize);
            t->button()->setFixedSize(Vector2i(mButtonSize,
                                               mButtonSize));

            t->setDrawConnections(mDrawConnections);
            t->setTreeIndent(mTreeIndent);
            t->setDisplayIndent(mDisplayIndent);
            t->setSubItemIndent(mSubItemIndent);
            t->setControlSpacing(mControlSpacing);
            t->setSubItemSpacing(mSubItemSpacing);
            t->setConnectionWidth(mConnectionWidth);
            t->recalculateLayout();
        }

        for( const auto c: w->children())
        {
            AdjustTree(c);
        }
    }
    

    Color mButtonFontColor  = Color(255, 255, 255, 255);
    Color mConnectionColor  = Color(255, 255, 255, 255);
    int mButtonSize         =  20;
    int mButtonFontSize     =  14;
    int mTreeIndent         =  8;
    int mDisplayIndent      =  8;
    int mSubItemIndent      = 16;
    int mControlSpacing     =  8;
    int mSubItemSpacing     =  4;
    double mConnectionWidth =  1.0;
    bool mDrawConnections   = true;
    
};


int
main(int, char **)
{
    nanogui::init();

    /* scoped variables */ {
        Screen *screen = nullptr;

        screen = new Screen(Vector2i(500, 800), "NanoGUI test");

        Window *window = new Window(screen, "Tree demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new BoxLayout(Orientation::Horizontal,
                                        Alignment::Minimum, 0, 6));


        new TreeControlPanel(window);

        Widget *tree = new Widget(window);
        tree->setLayout(new GroupLayout());


        new Label(tree, "A Tree", "sans-bold", 20);

        auto n1 = new TreeNode(tree);
        n1->setDisplay<Label>("node 1", "sans-bold", 72);
        new Label(n1, "node 1 1", "sans", 12);
        new Label(n1, "node 1 2", "sans", 14);
        new Label(n1, "node 1 3", "sans", 16);

        auto n2 = new TreeNode(n1);
        n2->setDisplay<Label>("node 2","sans-bold", 30);

        new Label(n2, "node 2 1", "sans", 12);
        new Label(n2, "node 2 2", "sans", 14);
        new Label(n2, "node 2 3", "sans", 16);
        new Label(n2, "node 2 4", "sans", 18);

        auto n3 = new TreeNode(n2);
        n3->setDisplay<Label>("node 3","sans-bold", 10);

        new Label(n3, "node 3 1", "sans", 12);
        new Label(n3, "node 3 2", "sans", 14);
        new Label(n3, "node 3 3", "sans", 10);
        new Label(n3, "node 3 4", "sans", 16);
        new Label(n3, "node 3 5", "sans", 18);

        new Label(n1, "node 1 4", "sans", 18);
        
        screen->setVisible(true);
        screen->performLayout();
        window->center();

        nanogui::mainloop();
    }

    nanogui::shutdown();
    return 0;
}
