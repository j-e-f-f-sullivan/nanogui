/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/hscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <string>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

class GLTexture {
public:
    using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
    GLTexture() = default;
    GLTexture(const std::string& textureName)
        : mTextureName(textureName), mTextureId(0) {}

    GLTexture(const std::string& textureName, GLint textureId)
        : mTextureName(textureName), mTextureId(textureId) {}

    GLTexture(const GLTexture& other) = delete;
    GLTexture(GLTexture&& other) noexcept
        : mTextureName(std::move(other.mTextureName)),
        mTextureId(other.mTextureId) {
        other.mTextureId = 0;
    }
    GLTexture& operator=(const GLTexture& other) = delete;
    GLTexture& operator=(GLTexture&& other) noexcept {
        mTextureName = std::move(other.mTextureName);
        std::swap(mTextureId, other.mTextureId);
        return *this;
    }
    ~GLTexture() noexcept {
        if (mTextureId)
            glDeleteTextures(1, &mTextureId);
    }

    GLuint texture() const { return mTextureId; }
    const std::string& textureName() const { return mTextureName; }

    /**
    *  Load a file in memory and create an OpenGL texture.
    *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
    */
    handleType load(const std::string& fileName) {
        if (mTextureId) {
            glDeleteTextures(1, &mTextureId);
            mTextureId = 0;
        }
        int force_channels = 0;
        int w, h, n;
        handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
        if (!textureData)
            throw std::invalid_argument("Could not load texture data from file " + fileName);
        glGenTextures(1, &mTextureId);
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        GLint internalFormat;
        GLint format;
        switch (n) {
            case 1: internalFormat = GL_R8; format = GL_RED; break;
            case 2: internalFormat = GL_RG8; format = GL_RG; break;
            case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
            case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
            default: internalFormat = 0; format = 0; break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return textureData;
    }

private:
    std::string mTextureName;
    GLuint mTextureId;
};

class ExampleApplication : public nanogui::Screen {
public:
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(1024, 768), "NanoGUI Test") {
        using namespace nanogui;

# if 0

        Window* window = new Window(this, "Basic widgets");
        window->setPosition(Vector2i(200, 15));
        window->setLayout(new GroupLayout());
#endif


        vector<pair<int, string>>
            icons = loadImageDirectory(mNVGContext, "icons");
        #if defined(_WIN32)
            string resourcesFolderPath("../resources/");
        #else
            string resourcesFolderPath("./");
        #endif
# if 0
        new Label(window, "Image panel & scroll panel", "sans-bold");
        PopupButton *imagePanelBtn = new PopupButton(window, "Image Panel");
        imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
        Popup *popup = imagePanelBtn->popup();
        VScrollPanel *vscroll = new VScrollPanel(popup);
        ImagePanel *imgPanel = new ImagePanel(vscroll);
        imgPanel->setImages(icons);
        popup->setFixedSize(Vector2i(245, 150));
#endif
        
        auto imageWindow = new Window(this, "Selected image");
        imageWindow->setPosition(Vector2i(710, 15));
        imageWindow->setLayout(new GroupLayout());

        // Load all of the images by creating a GLTexture object and saving the pixel data.
        for (auto& icon : icons) {
            GLTexture texture(icon.second);
            auto data = texture.load(resourcesFolderPath + icon.second + ".png");
            mImagesData.emplace_back(std::move(texture), std::move(data));
        }

        // Set the first texture
        auto imageView = new ImageView(imageWindow, mImagesData[0].first.texture());
        mCurrentImage = 0;
     
        imageView->setGridThreshold(20);
        imageView->setPixelInfoThreshold(20);
        imageView->setPixelInfoCallback(
            [this, imageView](const Vector2i& index) -> pair<string, Color> {
            auto& imageData = mImagesData[mCurrentImage].second;
            auto& textureSize = imageView->imageSize();
            string stringData;
            uint16_t channelSum = 0;
            for (int i = 0; i != 4; ++i) {
                auto& channelData = imageData[4*index.y()*textureSize.x() + 4*index.x() + i];
                channelSum += channelData;
                stringData += (to_string(static_cast<int>(channelData)) + "\n");
            }
            float intensity = static_cast<float>(255 - (channelSum / 4)) / 255.0f;
            float colorScale = intensity > 0.5f ? (intensity + 1) / 2 : intensity / 2;
            Color textColor = Color(colorScale, 1.0f);
            return { stringData, textColor };
        });

            // fix the width now, to force use of an h-scroll bar.
        imageWindow->setFixedWidth(imageWindow->preferredSize(mNVGContext).x());
        

        {
            HScrollPanel *hscroll = new HScrollPanel(imageWindow);
            ImagePanel *imgPanel  = new ImagePanel(hscroll);
            imgPanel->setImages(icons);
            imgPanel->setFitWidth(false);

            imgPanel->setCallback([this, imageView, imgPanel](int i) {
                imageView->bindImage(mImagesData[i].first.texture());
                mCurrentImage = i;
                cout << "Selected item " << i << '\n';
                });
        }
        
        performLayout();

        /* All NanoGUI widgets are initialized at this point. Now
           create an OpenGL shader to draw the main window contents.

           NanoGUI comes with a simple Eigen-based wrapper around OpenGL 3,
           which eliminates most of the tedious and error-prone shader and
           buffer object management.
        */

        
    }
    
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {

        /* Draw the user interface */
        Screen::draw(ctx);
    }

private:

    using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
    imagesDataType mImagesData;
    int mCurrentImage;
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        /* scoped variables */ {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}
