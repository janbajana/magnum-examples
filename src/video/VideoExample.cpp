/*
    This file is part of Magnum.

    Original authors — credit is appreciated but not required:

        2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 —
            Vladimír Vondruš <mosra@centrum.cz>

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to
    the public domain. We make this dedication for the benefit of the public
    at large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all
    present and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Corrade/Utility/Debug.h>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Resource.h>
#include <Corrade/Utility/Arguments.h>
#include <Corrade/Utility/DebugStl.h>

#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>

#include <Magnum/Video/AbstractImporter.h>

#include "TexturedTriangleShader.h"

namespace Magnum { namespace Examples {

class VideoExample: public Platform::Application {
    public:
        explicit VideoExample(const Arguments& arguments);

    private:
        void drawEvent() override;

        /* Video Importer */
        PluginManager::Manager<Video::AbstractImporter> _videoManager;
        Containers::Pointer<Video::AbstractImporter> _videoImporter;

        GL::Mesh _mesh;
        TexturedTriangleShader _shader;
        GL::Texture2D _texture;
};

VideoExample::VideoExample(const Arguments& arguments):
    Platform::Application{arguments, Configuration{}
        .setTitle("Magnum Textured Triangle Example")}
{
    Debug{} << "Starting video example.";
    Utility::Arguments args;
    args.addArgument("file").setHelp("file", "file to load")
        .addSkippedPrefix("magnum", "engine-specific options")
        .setGlobalHelp("Displays a 3D scene file provided on command line.")
        .parse(arguments.argc, arguments.argv);

    struct TriangleVertex {
        Vector2 position;
        Vector2 textureCoordinates;
    };
    const TriangleVertex data[]{
        {{-0.5f, -0.5f}, {0.0f, 0.0f}}, /* Left position and texture coordinate */
        {{ 0.5f, -0.5f}, {1.0f, 0.0f}}, /* Right position and texture coordinate */
        {{ 0.0f,  0.5f}, {0.5f, 1.0f}}  /* Top position and texture coordinate */
    };

    GL::Buffer buffer;
    buffer.setData(data);
    _mesh.setCount(3)
        .addVertexBuffer(std::move(buffer), 0,
            TexturedTriangleShader::Position{},
            TexturedTriangleShader::TextureCoordinates{});

    /* Load Video Importer Plugin */
    // PluginManager::Manager<Trade::AbstractImporter> videoManager;
    // Containers::Pointer<Trade::AbstractImporter> videoImporter =
    //     videoManager.loadAndInstantiate("GStVideoImporter");

    /* Load Video Importer Plugin */
    _videoImporter = _videoManager.loadAndInstantiate("GStVideoImporter");
    if(!_videoImporter) std::exit(1);

    Debug{} << "Opening file: " << args.value("file");

    /* Load file */
    if(!_videoImporter->openFile(args.value("file")))
        std::exit(4);

    if(_videoImporter->isOpened())
        _videoImporter->play();

    /* Load TGA importer plugin */
    PluginManager::Manager<Trade::AbstractImporter> manager;
    Containers::Pointer<Trade::AbstractImporter> importer =
        manager.loadAndInstantiate("TgaImporter");
    if(!importer) std::exit(1);

    /* Load the texture */
    const Utility::Resource rs{"video-data"};
    if(!importer->openData(rs.getRaw("stone.tga")))
        std::exit(2);

    /* Set texture data and parameters */
    Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
    CORRADE_INTERNAL_ASSERT(image);
    _texture.setWrapping(GL::SamplerWrapping::ClampToEdge)
        .setMagnificationFilter(GL::SamplerFilter::Linear)
        .setMinificationFilter(GL::SamplerFilter::Linear)
        .setStorage(1, GL::textureFormat(image->format()), image->size())
        .setSubImage(0, {}, *image);
}

void VideoExample::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

    using namespace Math::Literals;

    if(_videoImporter)
    {
            Debug{} << "playing ...";
    }

    _shader
        .setColor(0xffb2b2_rgbf)
        .bindTexture(_texture)
        .draw(_mesh);

    swapBuffers();
}

}}

MAGNUM_APPLICATION_MAIN(Magnum::Examples::VideoExample)
