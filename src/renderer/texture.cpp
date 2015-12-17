#include "texture.h"
#include "glutil.h"
#include <unordered_map>
#include <memory>
#include <algorithm>
#include "render_data.h"

namespace {

struct gSampler_t
{
    GLuint handle = 0;
};

struct gTexture_t
{
    bool srgb = false;
    bool mipmap = false;

    int32_t id = 0;
    GLuint handle = 0;
    GLuint maxMip = 0;
    GLsizei width = 0;
    GLsizei height = 0;
    GLsizei bpp = 0; // bpp is in bytes
    GLenum target;

    std::string name;

    std::vector< gTextureImage_t > texCoordSlots;

    glm::vec2 invRowPitch;

    ~gTexture_t( void )
    {
        if ( handle )
            GL_CHECK( glDeleteTextures( 1, &handle ) );
    }
};

using texturePointer_t = std::unique_ptr< gTexture_t >;

std::vector< texturePointer_t > gTextureMap;

INLINE gTexture_t* MakeTexture_GL( const gImageParams_t& canvasParams,
                         const gImageParams_t& slotParams,
                         const std::vector< gImageParams_t >& images )
{
    gTexture_t* tt = new gTexture_t();

    tt->target = GL_TEXTURE_2D;

    GL_CHECK( glGenTextures( 1, &tt->handle ) );
    GL_CHECK( glBindTexture( tt->target, tt->handle ) );

    GL_CHECK( glTexImage2D( tt->target, 0, canvasParams.internalFormat,
           canvasParams.width,
           canvasParams.height,
           0,
           canvasParams.format,
           GL_UNSIGNED_BYTE, nullptr ) );

    GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
    GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );

    GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
    GL_CHECK( glTexParameteri( tt->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );

    const uint32_t stride = uint32_t( canvasParams.width / slotParams.width );
    const uint32_t rows = uint32_t( canvasParams.height / slotParams.height );

    tt->texCoordSlots.resize( rows * stride );

    float invSlotWidth = 1.0f / ( float ) slotParams.width;
    float invSlotHeight = 1.0f / ( float ) slotParams.height;

    tt->invRowPitch.x = 1.0f / ( float ) stride;
    tt->invRowPitch.y = 1.0f / ( float ) rows;

    uint32_t y = 0;
    for ( uint32_t x = 0; x < images.size(); ++x )
    {
        uint32_t yb = y * slotParams.height;
        uint32_t xb = ( x % stride ) * slotParams.width;

        GL_CHECK( glTexSubImage2D( GL_TEXTURE_2D, 0, xb, yb, images[ x ].width,
            images[ x ].height, images[ x ].format, GL_UNSIGNED_BYTE, &images[ x ].data[ 0 ] ) );

        float fxStart = ( float )xb * invSlotWidth * tt->invRowPitch.x;
        float fyStart = ( float )yb * invSlotHeight * tt->invRowPitch.y;

        float fxEnd = fxStart + ( float )images[ x ].width * invSlotWidth * tt->invRowPitch.x;
        float fyEnd = fyStart + ( float )images[ x ].height * invSlotHeight * tt->invRowPitch.y;

        uint32_t slot = y * stride + ( x % stride );

        tt->texCoordSlots[ slot ].stOffsetStart = glm::vec2( fxStart, fyStart );
        tt->texCoordSlots[ slot ].stOffsetEnd = glm::vec2( fxEnd, fyEnd );
        tt->texCoordSlots[ slot ].imageScaleRatio = glm::vec2( ( float ) images[ x ].width * invSlotWidth,
                                                ( float ) images[ x ].height * invSlotHeight );

        if ( ( xb + slotParams.width ) % canvasParams.width == 0 )
            y++;
    }

    GL_CHECK( glBindTexture( tt->target, 0 ) );

    return tt;
}

} // end namespace

gTextureHandle_t GMakeTexture( const std::vector< gImageParams_t >& images, uint32_t flags )
{
    UNUSED( flags );

    glm::ivec2 maxDims( 0 );

    std::for_each( images.begin(), images.end(), [ &maxDims ]( const gImageParams_t& img )
    {
        if ( img.width > maxDims.x )
            maxDims.x = img.width;

        if ( img.height > maxDims.y )
            maxDims.y = img.height;
    });

    maxDims.x = int32_t( glm::pow( 2.0f, glm::ceil( glm::log2( ( float ) maxDims.x ) ) ) );
    maxDims.y = int32_t( glm::pow( 2.0f, glm::ceil( glm::log2( ( float ) maxDims.y ) ) ) );

    uint32_t closeSquare = uint32_t( glm::pow( 2.0f, glm::ceil( glm::log2( ( float )images.size() ) ) ) );
    uint32_t arrayDims = 2;

    while ( arrayDims * arrayDims < closeSquare )
        arrayDims += 2;

    // TODO: just make these extra imageParams ivec2 when passing to the make texture function...
    gImageParams_t canvasParams;
    canvasParams.width = maxDims.x * arrayDims;
    canvasParams.height = maxDims.y * arrayDims;

    gImageParams_t slotParams;
    slotParams.width = maxDims.x;
    slotParams.height = maxDims.y;

    gTexture_t* texture = MakeTexture_GL( canvasParams, slotParams, images );

    gTextureHandle_t handle =
    {
        ( uint32_t ) gTextureMap.size()
    };

    gTextureMap.push_back( std::move( texturePointer_t( texture ) ) );

    return handle;
}

void GFreeTexture( gTextureHandle_t& handle )
{
    if ( handle.id < gTextureMap.size() )
    {
        gTextureMap.erase( gTextureMap.begin() + handle.id );
    }

    handle.id = G_HANDLE_INVALID;
}

void GBindTexture( const gTextureHandle_t& handle )
{
    const gTexture_t* t = gTextureMap[ handle.id ].get();

    GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
    GL_CHECK( glBindTexture( t->target, t->handle ) );
}

void GReleaseTexture( const gTextureHandle_t& handle )
{
    const gTexture_t* t = gTextureMap[ handle.id ].get();

    GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
    GL_CHECK( glBindTexture( t->target, 0 ) );
}

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle, uint32_t slot )
{
    assert( handle.id < gTextureMap.size() );

    const gTexture_t* t = gTextureMap[ handle.id ].get();
    assert( slot < t->texCoordSlots.size() );

    return t->texCoordSlots[ slot ];
}

glm::vec2 GTextureInverseRowPitch( const gTextureHandle_t& handle )
{
    assert( handle.id < gTextureMap.size() );

    return gTextureMap[ handle.id ]->invRowPitch;
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
