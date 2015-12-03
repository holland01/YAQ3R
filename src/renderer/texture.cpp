#include "texture.h"
#include "glutil.h"
#include <unordered_map>
#include <memory>

namespace {

struct sampler_t
{
    GLuint handle = 0;
};

struct gTexture_t
{
    bool named = false;
    bool srgb = false;
    bool mipmap = false;

    GLuint handle = 0;
    GLuint maxMip = 0;
    GLsizei width = 0;
    GLsizei height = 0;
    GLsizei depth = 0;
    GLsizei bpp = 0; // bpp is in bytes

    std::string name;

    ~gTexture_t( void )
    {
        if ( handle )
        {
            GL_CHECK( glDeleteTextures( 1, &handle ) );
        }
    }
};

using texturePointer_t = std::unique_ptr< gTexture_t >;

std::unordered_map< std::string, texturePointer_t > gNamedTextures;
std::vector< texturePointer_t > gUnnamedTextures;

} // end namespace gl

gTexture_t* MakeTexture( const gImageParams_t& image )
{
    gTexture_t* tt = new gTexture_t();

    GL_CHECK( glGenTextures( 1, &tt->handle ) );
    GL_CHECK( glBindTexture( GL_TEXTURE_2D, tt->handle ) );
    GL_CHECK( glTexImage2D( GL_TEXTURE_2D,
        0, image.internalFormat, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, &image.data[ 0 ] ) );
    GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );

    return tt;
}

bool GSetImageBuffer( gImageParams_t& image, int32_t width, int32_t height, int32_t bpp, uint8_t fillValue )
{
    image.width = width;
    image.height = height;
    image.bpp = bpp;
    image.data.resize( width * height * bpp, fillValue );

    return GDetermineImageFormat( image );
}

bool GLoadImageFromFile( const std::string& imagePath, gImageParams_t& image )
{
    int32_t width, height, bpp;

    std::vector< uint8_t > tmp;
    if ( !File_GetPixels( imagePath, tmp, bpp, width, height ) )
        goto error;

    if ( bpp == 3 )
    {
        image.data.resize( width * height * 4, 255 );
        Pixels_24BitTo32Bit( &image.data[ 0 ], &tmp[ 0 ], width * height );
        bpp = 4;
    }
    else
    {
        image.data = std::move( tmp );
    }

    image.width = width;
    image.height = height;
    image.bpp = bpp;

    if ( !GDetermineImageFormat( image ) )
        goto error;

    return true;

error:
    MLOG_WARNING( "Image file attempted to load was \"%s\"", imagePath.c_str() );
    return false;
}

bool GDetermineImageFormat( gImageParams_t& image )
{
    switch( image.bpp )
    {
    case 1:
        image.format = G_BYTE_FORMAT;
        image.internalFormat = G_INTERNAL_BYTE_FORMAT;
        break;

    case 4:
        image.format = G_RGBA_FORMAT;
        image.internalFormat = G_INTERNAL_RGBA_FORMAT;
        break;
    default:
        MLOG_WARNING( "Unsupported image format of %i.", image.bpp );
        return false;
        break;
    }

    return true;
}

gTextureHandle_t GMakeTexture( const gImageParams_t& image, uint32_t flags )
{
    texturePointer_t tt( MakeTexture( image ) );

    gTexture_t* tmp = tt.get();

    gTextureHandle_t handle =
    {
        .flags = flags,
        .data = ( const void* )tmp
    };

    gUnnamedTextures.push_back( std::move( tt ) );

    return handle;
}

void GFreeTexture( const gTextureHandle_t& handle )
{
    gTexture_t* tex = ( gTexture_t* )handle.data;

    if ( tex )
    {
        if ( tex->named )
        {
            auto pair = gNamedTextures.find( tex->name );

            if ( pair != gNamedTextures.end() )
                gNamedTextures.erase( pair );
        }
    }
}
