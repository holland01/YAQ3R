#ifndef __GL_ATLAS_H__
#define __GL_ATLAS_H__

/*
 ───────────────▄████████▄────────
 ──────────────██▒▒▒▒▒▒▒▒██───────
 ─────────────██▒▒▒▒▒▒▒▒▒██───────
 ────────────██▒▒▒▒▒▒▒▒▒▒██───────
 ───────────██▒▒▒▒▒▒▒▒▒██▀────────
 ──────────██▒▒▒▒▒▒▒▒▒▒██─────────
 ─────────██▒▒▒▒▒▒▒▒▒▒▒██─────────
 ────────██▒████▒████▒▒██─────────
 ────────██▒▒▒▒▒▒▒▒▒▒▒▒██─────────
 ────────██▒────▒▒────▒██─────────
 ────────██▒██──▒▒██──▒██─────────
 ────────██▒────▒▒────▒██─────────
 ────────██▒▒▒▒▒▒▒▒▒▒▒▒██─────────
 ───────██▒▒▒████████▒▒▒▒██───────
 ─────██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██─────
 ───██▒▒██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██▒▒██───
 ─██▒▒▒▒██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██▒▒▒▒██─
 █▒▒▒▒██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██▒▒▒▒█
 █▒▒▒▒██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒██▒▒▒▒█
 █▒▒████▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒████▒▒█
 ▀████▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒████▀
 ──█▌▌▌▌▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▌▌▌███──
 ───█▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌█────
 ───█▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌█────
 ────▀█▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌▌██▀─────
 ─────█▌▌▌▌▌▌████████▌▌▌▌▌██──────
 ──────██▒▒██────────██▒▒██───────
 ──────▀████▀────────▀████▀───────

 If you're not using GL ES 2.0 or old school
 GL 2.x you're probably wasting your time with this:
 just use GL_TEXTURE_2D_ARRAY, wash your hands,
 and be done with it :D
 */

#include <stdio.h>
#include <dirent.h>

#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <stack>
#include <sstream>
#include <array>
#include <memory>
#include <unordered_map>
#include <utility>
#include <thread>

#include "stb_image.h"

#define GL_ATLAS_TEX_FORMAT GL_RGBA

#ifdef GL_ATLAS_GLEW
	#include <GL/glew.h>
	#include <GLFW/glfw3.h>
	#define GL_ATLAS_INTERNAL_TEX_FORMAT GL_RGBA8
#elif defined(GL_ATLAS_EGL)
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#include <EGL/egl.h>
	#define GL_ATLAS_INTERNAL_TEX_FORMAT GL_RGBA
#else
	#error "define either GL_ATLAS_GLEW or GL_ATLAS_EGL to " \
		"choose header implementation"
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef GL_ATLAS_MAIN
	#define SHADER(s) "#version 410 core\n"#s
	#define SS_INDEX(s) "[" << (s) << "]"
#endif

//------------------------------------------------------------------------------------
// logging and GL error handling
//------------------------------------------------------------------------------------
#define ga_inline inline

namespace gla {

	static ga_inline void atlas_error_exit(void)
	{
#ifdef GL_ATLAS_MAIN
		g_running = false;
#else
		exit(1);
#endif
	}

	static std::vector<std::string> g_gl_err_msg_cache;

	static ga_inline void exit_on_gl_error(int line, const char* func,
		const char* expr)
	{
		GLenum err = glGetError();

		if (err != GL_NO_ERROR) {
			char msg[256];
			memset(msg, 0, sizeof(msg));

#ifdef GL_ATLAS_GLEW
			sprintf(
				&msg[0],
				"GL_ATLAS_ERROR (%x) in %s@%i [%s]: %s\n",
				err,
				func,
				line,
				expr,
				(const char* )gluErrorString(err));
#else // No error string implementation at the moment - some day...
			sprintf(
				&msg[0],
				"GL_ATLAS_ERROR (%x) in %s@%i [%s]\n",
				err,
				func,
				line,
				expr);
#endif

			std::string smsg(msg);
			if (std::find(g_gl_err_msg_cache.begin(), g_gl_err_msg_cache.end(), smsg)
				== g_gl_err_msg_cache.end()) {
				printf("%s", smsg.c_str());
				g_gl_err_msg_cache.push_back(smsg);
			}

			atlas_error_exit();
		}
	}

	static ga_inline void logf_impl( int line, const char* func,
		const char* fmt, ... )
	{
		va_list arg;

		va_start( arg, fmt );
		fprintf( stdout, "\n[ %s@%i ]: ", func, line );
		vfprintf( stdout, fmt, arg );
		fputs( "\n", stdout );
		va_end( arg );
	}

#ifdef GLA_DEBUG
	#define IF_DEBUG(expr) expr
#else
	#define IF_DEBUG(expr)
#endif

#ifdef GLA_IO
	#define gla_logf(...) logf_impl(__LINE__, __FUNCTION__, __VA_ARGS__)
#else
	#define gla_logf(...)
#endif

#ifdef GLA_DEBUG_RENDERER
	#define GL_H(expr) \
	do { \
		( expr ); \
		exit_on_gl_error(__LINE__, __FUNCTION__, #expr); \
	} while (0)
#else
	#define GL_H(expr) expr
#endif

#define DESIRED_BPP 4

//-------------------------------------------------------------------------
// atlas generation-specific classes/functions.
//-------------------------------------------------------------------------

	template <class numType>
	numType next_power2(numType x)
	{
		x--;
		x = x | x >> 1;
		x = x | x >> 2;
		x = x | x >> 4;
		x = x | x >> 8;
		x = x | x >> 16;
		x++;
		return x;
	}

	// When we have multiple atlasses for a single set of images, we use layers.

	static void ga_inline alloc_blank_texture(
		size_t width,
		size_t height,
		uint32_t clear_val);

	using image_fill_map_t = std::unordered_map<uint16_t, uint8_t>;

	struct atlas_image_info_t {
		uint8_t 	layer;
		glm::vec2 	coords;
		glm::vec2 	inverse_layer_dims;
	};

	struct atlas_t {
		uint32_t num_images;

		uint32_t area_accum;

		std::vector<uint8_t> layers;

		std::vector<uint16_t> widths;
		std::vector<uint16_t> heights;

		std::vector<uint16_t> dims_x;
		std::vector<uint16_t> dims_y;

		std::vector<uint16_t> coords_x;
		std::vector<uint16_t> coords_y;

		std::vector<GLuint> layer_tex_handles;

		std::vector<std::vector<uint8_t>> buffer_table;

		std::vector<std::string> filenames; // optional

		std::unordered_map<size_t, uint16_t> key_map;	// optional

		uint16_t origin_x(uint16_t image) const
		{
			return coords_x[image];
		}

		uint16_t origin_y(uint16_t image) const
		{
			return coords_y[image];
		}

		uint8_t layer(uint16_t image) const
		{
			assert(image < layers.size());
			assert(layers[image] != 0xFF);
			return layers[image];
		}

		atlas_image_info_t image_info(uint16_t image) const
		{
			uint8_t L = layer(image);

			atlas_image_info_t img {
				L,
				glm::vec2(origin_x(image), origin_y(image)),
				glm::vec2(
					1.0f / static_cast<float>(widths[L]),
					1.0f / static_cast<float>(heights[L])
				)
			};

			return img;
		}

		void push_layer(uint16_t width, uint16_t height)
		{
			size_t index = layer_tex_handles.size();

			layer_tex_handles.push_back(0);
			widths.push_back(width);
			heights.push_back(height);

			GL_H( glGenTextures(1, &layer_tex_handles[index]) );

			bind(index);

			GL_H( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR) );
			GL_H( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR) );
			GL_H( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				GL_CLAMP_TO_EDGE) );
			GL_H( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				GL_CLAMP_TO_EDGE) );

			alloc_blank_texture(widths[index], heights[index], 0x00000000);

			release();
		}

		void set_layer(uint16_t image, uint8_t layer)
		{
			if (layers.size() != num_images)
				layers.resize(num_images, 0xFF);

			layers[image] = layer;
		}

		void bind(uint8_t layer) const
		{
			GL_H( glBindTexture(GL_TEXTURE_2D, layer_tex_handles[layer]) );
		}

		void bind_to_active_slot(uint8_t layer, GLenum offset) const
		{
			GL_H( glActiveTexture(GL_TEXTURE0 + offset) );
			bind(layer);
		}

		void release(void) const
		{
			GL_H( glBindTexture(GL_TEXTURE_2D, 0) );
		}

		void release_from_active_slot(GLenum offset) const
		{
			GL_H( glActiveTexture(GL_TEXTURE0 + offset) );
			release();
		}

		void write_origins(uint16_t image, uint16_t x, uint16_t y)
		{
			if (coords_x.size() != num_images)
				coords_x.resize(num_images, 0);

			coords_x[image] = x;

			if (coords_y.size() != num_images)
				coords_y.resize(num_images, 0);

			coords_y[image] = y;
		}

		void fill_atlas_image(size_t image)
		{
			GL_H( glTexSubImage2D(	GL_TEXTURE_2D,
									0,
									(GLsizei) origin_x(image),
									(GLsizei) origin_y(image),
									dims_x[image],
									dims_y[image],
									GL_ATLAS_TEX_FORMAT,
									GL_UNSIGNED_BYTE,
									&buffer_table[image][0]) );
		}

		uint16_t key_image(size_t key) const
		{
			return key_map.at(key);
		}

		void map_key_to_image(size_t key, uint16_t image)
		{
			key_map[key] = image;
		}

		void free_memory(void)
		{
			{
				GLint curr_bound_tex;
				GL_H( glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_bound_tex) );

				// We unbind if any of this atlas's textures are bound
				// because a glDelete call on a bound item can't be fulfilled
				// until that item is unbound

				bool bound = false;
				for (GLuint handle = 0; handle < layer_tex_handles.size()
					&& !bound && curr_bound_tex; ++handle) {
					bound = (GLuint)curr_bound_tex == layer_tex_handles[handle];
				}

				if (bound) {
					GL_H( glBindTexture(GL_TEXTURE_2D, 0) );
				}

				GL_H( glDeleteTextures(layer_tex_handles.size(),
					&layer_tex_handles[0]) );
			}

			num_images = 0;
			area_accum = 0;

			widths.clear();
			heights.clear();
			dims_x.clear();
			dims_y.clear();
			coords_x.clear();
			coords_y.clear();
			buffer_table.clear();
			filenames.clear();

			layers.clear();
			layer_tex_handles.clear();
		}

		~atlas_t(void)
		{
			free_memory();
		}

		atlas_t(void)
			: 	num_images(0),
				area_accum(0)
		{}
	};

	//------------------
	// gen_layer_bsp
	//
	// generates a layer (think of "layer" in this sense as just a texture
	// representing a portion for a set of a group of images ).
	// The core algorithm is based on 2D BSP generation.
	//
	// it assesses whether or not sorted images for this layer
	// actually save space and how large this layer actually needs to be.
	//
	// the set of images a layer has will be more efficiently placed if the
	// variation of the image sizes is high; if a layer consists
	// of images which are of the same dimensions, then there will be a lot of unused
	// space.
	//
	// idea behind BSP algol is from here:
	// http://gamedev.stackexchange.com/a/34193
	//------------------

	class gen_layer_bsp
	{
		struct node_t;

		using node_ptr_t = std::unique_ptr<node_t, void (*)(node_t*)>;
		using atlas_type_t = atlas_t;

		atlas_type_t& atlas;

		// one for a sorted atlas, the other for an unsorted atlas. The atlas
		// which takes the least amount of space is the winner
		node_ptr_t root;
		glm::ivec3 layer_dims;

		// Only left child's are capable of storing image indices,
		// from the perspective of the child's parent.

		// The "lines" (expressed implicitly) will only have
		// positive normals that face either to the right, or upward.

		struct node_t {
			bool region;
			int32_t image;

			glm::ivec2 origin;
			glm::ivec2 dims;

			node_t* left_child;
			node_t* right_child;

			node_t(void)
			:   region(false),
				image(-1),
				origin(0, 0), dims(0, 0),
				left_child(nullptr), right_child(nullptr)
			{}

			static void destroy(node_t* n)
			{
				if (n) {
					destroy(n->left_child);
					destroy(n->right_child);
					delete n;
				}
			}
		};

		node_t* insert_node(node_t* node, uint16_t image)
		{
			if (node->region) {
				node_t* n = insert_node(node->left_child, image);

				if (n) {
					node->left_child = n;
					return node;
				}

				n = insert_node(node->right_child, image);

				if (n) {
					node->right_child = n;
					return node;
				}
			} else {

				if (node->image >= 0)
					return nullptr;

				glm::ivec2 image_dims(atlas.dims_x[image], atlas.dims_y[image]);

				if (node->dims.x < image_dims.x || node->dims.y < image_dims.y)
					return nullptr;

				if (node->dims.x == image_dims.x
					&& node->dims.y == image_dims.y) {

					node->image = image;

					if ((node->origin.x + image_dims.x) > layer_dims.x)
						layer_dims.x = node->origin.x + image_dims.x;

					if ((node->origin.y + image_dims.y) > layer_dims.y)
						layer_dims.y = node->origin.y + image_dims.y;

					assert(layer_dims.x <= root->dims.x);
					assert(layer_dims.y <= root->dims.y);

					atlas.write_origins(node->image, node->origin.x,
						node->origin.y);

					return node;
				}

				node->region = true;

				uint16_t dx = node->dims.x - image_dims.x;
				uint16_t dy = node->dims.y - image_dims.y;

				node->left_child = new node_t();
				node->right_child = new node_t();

				// Is the partition line vertical?
				if (dx > dy) {
					node->left_child->dims.x = image_dims.x;
					node->left_child->dims.y = node->dims.y;
					node->left_child->origin = node->origin;

					node->right_child->dims.x = dx;
					node->right_child->dims.y = node->dims.y;
					node->right_child->origin = node->origin;

					node->right_child->origin.x += image_dims.x;

				// Nope, it's horizontal
				} else {
					node->left_child->dims.x = node->dims.x;
					node->left_child->dims.y = image_dims.y;
					node->left_child->origin = node->origin;

					node->right_child->dims.x = node->dims.x;
					node->right_child->dims.y = dy;
					node->right_child->origin = node->origin;

					node->right_child->origin.y += image_dims.y;
				}

				// The only way we're able to make it here
				// is if node's dimensions are >= the image's dimensions.
				// If they're exactly equal, then node would have already been
				// set and this call won't happen.
				// Otherwise, the left child's values are set to values
				// which have already been examined for size, or are
				// set to one of the image's dimension values.

				node->left_child = insert_node(node->left_child, image);

				assert(node->left_child);

				return node;
			}

			return nullptr;
		}

		bool insert(uint16_t image)
		{
			node_t* res = insert_node(root.get(), image);

			return !!res;
		}

	public:

		const glm::ivec3& dims(void) const
		{
			return layer_dims;
		}

		gen_layer_bsp(atlas_type_t& atlas_, image_fill_map_t& image_check)
			:   atlas(atlas_),
				root(new node_t(), node_t::destroy)
		{
			// Setup some upper bounds for the width/height values
			{
				GLint max_dims;
				GL_H( glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_dims) );

				uint32_t root_area_accumf =
					next_power2((uint32_t) glm::sqrt((float) atlas.area_accum));

				if ((uint32_t) max_dims > root_area_accumf)
					max_dims = (GLint) root_area_accumf;

				root->dims = glm::ivec2(max_dims, max_dims);
			}

			std::vector<uint16_t> sorted(image_check.size(), 0);

			uint16_t i = 0;

			for (auto image: image_check)
				sorted[i++] = image.first;

			std::sort(sorted.begin(), sorted.end(), [this](uint16_t a,
				uint16_t b) -> bool {
				if (atlas.dims_x[a] == atlas.dims_x[b]) {
					return atlas.dims_y[a] > atlas.dims_y[b];
				}
				return atlas.dims_x[a] > atlas.dims_x[b];
			});

			for (uint16_t image: sorted) {
				if (insert(image)) {
					image_check[image] = 1;
					layer_dims[2] += 1;
				}
			}
		}
	};

	//------------------------------------------------------------------------------------
	// minor texture utils
	//------------------------------------------------------------------------------------
	static ga_inline void alloc_blank_texture(size_t width, size_t height,
									uint32_t clear_val)
	{
		std::vector<uint32_t> blank(width * height, clear_val);
		GL_H( glTexImage2D(	GL_TEXTURE_2D,
							0,
							GL_ATLAS_INTERNAL_TEX_FORMAT,
							(GLsizei) width,
							(GLsizei) height,
							0,
							GL_ATLAS_TEX_FORMAT,
							GL_UNSIGNED_BYTE,
							&blank[0]) );
	}


	static ga_inline void convert_rgb_to_rgba(uint8_t* dest,
		const uint8_t* src, size_t dim_x, size_t dim_y)
	{
		for (size_t y = 0; y < dim_y; ++y) {
			for (size_t x = 0; x < dim_x; ++x) {
				size_t i = y * dim_x + x;
				dest[i * 4 + 0] = src[i * 3 + 0];
				dest[i * 4 + 1] = src[i * 3 + 1];
				dest[i * 4 + 2] = src[i * 3 + 2];
				dest[i * 4 + 3] = 255;
			}
		}
	}

	static ga_inline uint32_t pack_rgba(uint8_t* rgba)
	{
		return (((uint32_t)rgba[0]) << 0)
		| (((uint32_t)rgba[1]) << 8)
		| (((uint32_t)rgba[2]) << 16)
		| (((uint32_t)rgba[3]) << 24);
	}

	static ga_inline void unpack_rgba(uint8_t* dest, uint32_t src)
	{
		dest[0] = (src >> 0) & 0xFF;
		dest[1] = (src >> 8) & 0xFF;
		dest[2] = (src >> 16) & 0xFF;
		dest[3] = (src >> 24) & 0xFF;
	}

	static ga_inline void flip_rows_rgba(uint8_t* image_data,
		size_t dim_x, size_t dim_y)
	{
		size_t half_dy = dim_y >> 1;
		for (size_t y = 0; y < half_dy; ++y) {
			for (size_t x = 0; x < dim_x; ++x) {
				size_t top_x = (y * dim_x + x) * 4;
				size_t bot_x = ((dim_y - y - 1) * dim_x + x) * 4;
				uint32_t top = pack_rgba(&image_data[top_x]);

				unpack_rgba(&image_data[top_x],
							pack_rgba(&image_data[bot_x]));

				unpack_rgba(&image_data[bot_x], top);
			}
		}
	}

	//------------------------------------------------------------------------------------
	// gen
	//------------------------------------------------------------------------------------

	static ga_inline void gen_atlas_layers(atlas_t& atlas)
	{
		// Basic idea is to keep track of each image
		// and the layer it belongs to; every image
		// which has yet to be assigned to a layer
		// remains in this map after a given iteration
		image_fill_map_t global_unfill;
		for (uint16_t i = 0; i < atlas.num_images; ++i) {
			global_unfill[i];
		}

		uint8_t layer = 0;

		while (!global_unfill.empty()) {
			image_fill_map_t local_fill;

			local_fill.insert(global_unfill.begin(),
							  global_unfill.end());

			// gen_layer_bsp allocates a fair amount of memory
			// internally, so it's best to just wrap it in an
			// inner block since we have plenty of processing to do
			// afterward
			uint16_t w, h;
			{
				gen_layer_bsp placed(atlas, local_fill);

				const glm::ivec3& dims = placed.dims();

				w = next_power2(dims[0]);
				h = next_power2(dims[1]);
			}

			atlas.push_layer(w, h);

			atlas.bind(layer);

			for (auto& image: local_fill) {
				if (image.second) {
					atlas.set_layer(image.first, layer);
					atlas.fill_atlas_image(image.first);
					global_unfill.erase(image.first);
				}
			}

			atlas.release();

			layer++;
		}

		gla_logf("Total Images: %lu\nArea Accum: %lu",
			 atlas.num_images, atlas.area_accum);

		for (uint32_t i = 0; i < atlas.widths.size(); ++i) {
			gla_logf("Layer Size [%i/%i]: %i x %i",
				i + 1,
				atlas.widths.size(),
				atlas.widths[i],
				atlas.heights[i]);
		}
	}

	static ga_inline void push_atlas_image(atlas_t& atlas,
		uint8_t* buffer, int dx, int dy, int bpp, bool flip = true)
	{
		std::vector<uint8_t> image_data(dx * dy * DESIRED_BPP, 0);

		if (bpp == 3) {
			convert_rgb_to_rgba(&image_data[0], buffer, dx, dy);
		} else if (bpp == DESIRED_BPP) {
			memcpy(&image_data[0], buffer, dx * dy * DESIRED_BPP);
		} else {
			gla_logf("ERROR: received image of would-be index %i" \
			"that does not contain a supported bytes per pixel count."\
			" Dimensions: %i x %i. BPP received: %i",
			(int) atlas.num_images, dx, dy, bpp);
			atlas_error_exit();
			return;
		}

		atlas.area_accum += dx * dy;

		atlas.dims_x.push_back(dx);
		atlas.dims_y.push_back(dy);

		// stb_image treats the image origin as upper left and OpenGL doesn't.
		if ( flip ) {
			flip_rows_rgba(&image_data[0], dx, dy);
		}

		atlas.buffer_table.push_back(std::move(image_data));
		atlas.num_images++;
	}

	static ga_inline void make_atlas_from_dir(
		atlas_t& atlas,
		std::string dirpath)
	{
		if (*(dirpath.end()) != '/')
			dirpath.append(1, '/');

		DIR* dir = opendir(dirpath.c_str());
		struct dirent* ent = NULL;

		if (!dir) {
			gla_logf("Could not open %s", dirpath.c_str());
			atlas_error_exit();
			return;
		}

		assert(DESIRED_BPP == 4
			&& "Code is only meant to work with textures using desired bpp of 4!");

		atlas.free_memory();

		while (!!(ent = readdir(dir))) {
			std::string filepath(dirpath);
			filepath.append(ent->d_name);

			int dx, dy, bpp;
			stbi_uc* stbi_buffer = stbi_load(filepath.c_str(), &dx, &dy, &bpp,
											 STBI_default);

			if (!stbi_buffer) {
				gla_logf("Warning: could not open %s. Skipping.", filepath.c_str());
				continue;
			}

			if (bpp != DESIRED_BPP && bpp != 3) {
				gla_logf("Warning: found invalid bpp value of %i for %s. Skipping.",
					 bpp, filepath.c_str());
				continue;
			}

			atlas.filenames.push_back(std::string(ent->d_name));

			push_atlas_image(atlas, stbi_buffer, dx, dy, bpp);

			stbi_image_free(stbi_buffer);
		}

		closedir(dir);

		gen_atlas_layers(atlas);
	}

} // namespace gla

#endif
