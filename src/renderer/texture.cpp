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

struct gTexSlot_t
{
    glm::vec2 stOffsetStart;
    glm::vec2 stOffsetEnd;
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

    std::string name;

    std::vector< gTexSlot_t > texCoordSlots;

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

    GL_CHECK( glGenTextures( 1, &tt->handle ) );
    GL_CHECK( glBindTexture( GL_TEXTURE_2D, tt->handle ) );

    GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, canvasParams.internalFormat,
           canvasParams.width,
           canvasParams.height,
           0,
           canvasParams.format,
           GL_UNSIGNED_BYTE, &canvasParams.data[ 0 ] ) );

    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) );
    GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) );

    const uint32_t stride = uint32_t( canvasParams.width / slotParams.width );
    const uint32_t rows = uint32_t( canvasParams.height / slotParams.height );

    tt->texCoordSlots.resize( rows * stride );

    float invByteWidth = 1.0f / ( float ) slotParams.width;
    float invByteHeight = 1.0f / ( float ) slotParams.height;

    float invStride = 1.0f / ( float ) stride;
    float invRowCount = 1.0f / ( float ) rows;

    uint32_t y = 0;
    for ( uint32_t x = 0; x < images.size(); ++x )
    {
        uint32_t yb = y * slotParams.height;
        uint32_t xb = ( x % stride ) * slotParams.width;

        GL_CHECK( glTexSubImage2D( GL_TEXTURE_2D, 0, xb, yb, images[ x ].width,
            images[ x ].height, images[ x ].format, GL_UNSIGNED_BYTE, &images[ x ].data[ 0 ] ) );

        float fxStart = ( float )xb * invStride;
        float fyStart = ( float )yb * invRowCount;

        float fxEnd = fxStart + ( float )images[ x ].width * invByteWidth * invStride;
        float fyEnd = fyStart + ( float )images[ x ].height * invByteHeight * invRowCount;

        tt->texCoordSlots[ y * stride + x ].stOffsetStart = glm::vec2( fxStart, fyStart );
        tt->texCoordSlots[ y * stride + x ].stOffsetEnd = glm::vec2( fxEnd, fyEnd );

        if ( ( xb + slotParams.width ) % canvasParams.width == 0 )
            y++;
    }

    GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );

    return tt;
}

struct gVertexBuffer_t
{
    int32_t id = 0;
    GLuint handle = 0;

    ~gVertexBuffer_t( void )
    {
        DeleteBufferObject( GL_ARRAY_BUFFER, handle );
    }
};

using vertexBufferPointer_t = std::unique_ptr< gVertexBuffer_t >;

std::vector< vertexBufferPointer_t > gVertexBufferMap;

INLINE std::vector< bspVertex_t > ConvertToDrawVertex( const std::vector< glm::vec3 >& vertices )
{
    std::vector< bspVertex_t > bufferData;

    for ( const auto& v: vertices )
    {
        bspVertex_t vt;
        vt.position = v;
        vt.color = glm::vec4( 1.0f );
        vt.normal = glm::vec3( 1.0f );
        vt.texCoords[ 0 ] = glm::vec2( 0.0f );
        vt.texCoords[ 1 ] = glm::vec2( 0.0f );
        bufferData.push_back( vt );
    }

    return std::move( bufferData );
}

INLINE gVertexBuffer_t* MakeVertexBuffer_GL( const std::vector< bspVertex_t >& bufferData )
{
    gVertexBuffer_t* buffer = new gVertexBuffer_t();

    GL_CHECK( glGenBuffers( 1, &buffer->handle ) );
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, buffer->handle ) );
    GL_CHECK( glBufferData( GL_ARRAY_BUFFER, bufferData.size() * sizeof( bspVertex_t ), &bufferData[ 0 ].position[ 0 ], GL_STATIC_DRAW ) );
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

    return buffer;
}

} // end namespace

void GEnableDepthBuffer( void )
{
    GL_CHECK( glEnable( GL_DEPTH_TEST ) );
    GL_CHECK( glDepthFunc( GL_LEQUAL ) );
    GL_CHECK( glClearDepth( 1.0f ) );
}

gVertexBufferHandle_t GMakeVertexBuffer( const std::vector< glm::vec3 >& vertices )
{
    gVertexBuffer_t* buffer = MakeVertexBuffer_GL( ConvertToDrawVertex( vertices ) );

    gVertexBufferHandle_t handle =
    {
        .id = ( uint32_t ) gVertexBufferMap.size()
    };

    gVertexBufferMap.push_back( std::move( vertexBufferPointer_t( buffer ) ) );

    return handle;
}

void GFreeVertexBuffer( gVertexBufferHandle_t& handle )
{
    if ( handle.id < gVertexBufferMap.size() )
        gVertexBufferMap.erase( gVertexBufferMap.begin() + handle.id );

    handle.id = G_HANDLE_INVALID;
}

void GBindVertexBuffer( const gVertexBufferHandle_t& buffer )
{
    if ( buffer.id < gVertexBufferMap.size() )
        GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, gVertexBufferMap[ buffer.id ]->handle ) );
}

void GReleaseVertexBuffer( void )
{
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
}

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

    maxDims.x = int32_t( glm::pow( 2, glm::ceil( glm::log2( ( float ) maxDims.x ) ) ) );
    maxDims.y = int32_t( glm::pow( 2, glm::ceil( glm::log2( ( float ) maxDims.y ) ) ) );

    uint32_t closeSquare = uint32_t( glm::pow( 2, glm::ceil( glm::log2( ( float )images.size() ) ) ) );
    uint32_t arrayDims = 2;

    while ( arrayDims * arrayDims < closeSquare )
        arrayDims += 2;

    arrayDims *= arrayDims;

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
        .id = ( uint32_t ) gTextureMap.size()
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
