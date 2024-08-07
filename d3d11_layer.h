
//WHY DOES THIS GET PUT INSIDE THE PRECOMPILED HEADER
#include <d3d11.h>
#include <dxgi1_3.h>
#include <D3DCompiler.h>
#include <DirectXMath.h>

#if DEBUGMODE
	#include <dxgidebug.h>
#endif

using namespace DirectX; //TODO: THIS IS ONLY USED IN directxmath.h

#define PS_PROFILE "ps_5_0"
#define VS_PROFILE "vs_5_0"

#if DEBUGMODE
	#define DX11_CREATE_DEVICE_DEBUG_FLAG D3D11_CREATE_DEVICE_DEBUG
#else
	#define DX11_CREATE_DEVICE_DEBUG_FLAG 0
#endif

typedef ID3D11ShaderResourceView 		Dx11_texture_view;
typedef ID3D11InputLayout 					Dx11_input_layout;
typedef ID3D11Device 						Dx11_device;
typedef ID3D11DeviceContext				Dx11_device_context;
typedef IDXGISwapChain1						Dx11_swap_chain;
typedef ID3D11RasterizerState				Dx11_rasterizer_state;
typedef ID3D11SamplerState					Dx11_sampler_state;
typedef ID3D11RenderTargetView			Dx11_render_target_view;
typedef ID3D11Texture2D						Dx11_texture2d;


typedef ID3D11Buffer 						Dx11_buffer;
typedef D3D11_BUFFER_DESC					Dx11_buffer_desc;
typedef D3D11_SUBRESOURCE_DATA			Dx11_subresource_data;

typedef ID3D11VertexShader					Dx11_vertex_shader;
typedef D3D11_INPUT_ELEMENT_DESC			Dx11_input_layout_desc;
typedef ID3D11PixelShader 					Dx11_pixel_shader;

typedef ID3DBlob								Dx11_blob;
typedef ID3D11Resource 						Dx11_resource;

typedef D3D11_SAMPLER_DESC					Dx11_sampler_desc;
typedef D3D11_RASTERIZER_DESC				Dx11_rasterizer_desc;
typedef ID3D11BlendState					Dx11_blend_state;
typedef ID3D11DepthStencilState			Dx11_depth_stencil_state;
typedef D3D11_DEPTH_STENCIL_DESC			Dx11_depth_stencil_desc;
typedef ID3D11ShaderResourceView			Dx11_texture_view;
typedef ID3D11DepthStencilView 			Dx11_depth_stencil_view;
typedef D3D11_VIEWPORT						Dx11_viewport;


struct D3D
{
	Dx11_device* device;
	Dx11_device_context* context;
	Dx11_swap_chain* swap_chain; // maybe change to IDXGISwapChain1 
	Dx11_viewport viewport;

	Dx11_rasterizer_state* rasterizer_state;
	Dx11_sampler_state* sampler;
	
	// ID3D11Texture2D* pre_processing_render_target_texture;
	// ID3D11Texture2D* depth_render_target_texture;
	// Dx11_texture2d* render_target_textures_list[4];

	// Dx11_render_target_view* render_target_views_list[4];

	// Dx11_texture_view* render_target_texture_views_list[4];
};

struct D3D_constant_buffer
{
	Dx11_buffer* buffer;
	u32 size;
	Shader_constant_buffer_register_index register_index;
};

struct Render_target
{
	Dx11_texture2d* texture;
	Dx11_render_target_view* target_view;
	Dx11_texture_view** texture_view;
};

// struct Dx11_render_pipeline
// {
// 	Dx11_vertex_shader* vs;
// 	Dx11_input_layout* input_layout;
// 	Dx11_pixel_shader* ps;
// 	Dx11_blend_state* blend_state;
// 	Dx11_depth_stencil_state* depth_stencil_state;
// 	Dx11_depth_stencil_view* depth_stencil_view;

// 	Dx11_texture_view* default_texture_view; 
// };

struct Dx_mesh
{
	u32 vertex_size;
	u32 vertices_count;//TODO: rename this to vertex_count
	u32 indices_count;//TODO: rename this to index_count

	Dx11_buffer* vertex_buffer;
	Dx11_buffer* index_buffer;

	D3D11_PRIMITIVE_TOPOLOGY topology;
};

//where should this be defined?
struct Vertex_shader
{
	String filename; // this is only used in debug mode for shader hot reloading
	FILETIME last_write_time;

	Dx11_vertex_shader* shader;
	Dx11_input_layout* input_layout;
};

struct Pixel_shader
{
	String filename; // this is only used in debug mode for shader hot reloading
	FILETIME last_write_time;

	Dx11_pixel_shader* shader;
};

struct Depth_stencil
{
	Dx11_depth_stencil_state* state;
	Dx11_depth_stencil_view* view;
};


internal Dx_mesh
dx11_init_mesh(D3D* dx, void* vertices, u32 v_count, int v_size, u16* indices, u32 i_count, D3D11_PRIMITIVE_TOPOLOGY topology)
{
	Dx_mesh result = {0};
	// VERTEX BUFFER
	D3D11_BUFFER_DESC bd = {0};
	bd.ByteWidth        = v_count * v_size;
	bd.Usage            = D3D11_USAGE_DEFAULT;
	bd.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA init_data = {0};
	init_data.pSysMem            = vertices;

	ASSERTHR(dx->device->CreateBuffer( &bd, &init_data, &result.vertex_buffer));

	// INDEX BUFFER
	bd = {0};
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.ByteWidth = i_count*sizeof(u16);
	bd.StructureByteStride = sizeof(u16);
	init_data = {0};
	init_data.pSysMem = indices;
	ASSERTHR(dx->device->CreateBuffer(&bd, &init_data, &result.index_buffer));
	result.topology = topology;
	result.vertices_count = v_count;
	result.vertex_size = v_size;
	result.indices_count = i_count;
	return result;
}
internal Dx_mesh
dx11_init_mesh(D3D* dx, Mesh_primitive* primitives, D3D11_PRIMITIVE_TOPOLOGY topology)
{
	return dx11_init_mesh(dx, 
		primitives->vertices, primitives->vertex_count, primitives->vertex_size, 
		primitives->indices, primitives->indices_count,
		topology
	);
}

internal File_data
dx11_get_compiled_shader(String filename, Memory_arena* arena, char* entrypoint_name, char* target_profile)
{
	File_data source_shaders_file = win_read_file(filename, arena);

	D3D_SHADER_MACRO defines[] = {
		NULL, NULL
	};
	// D3D_FEATURE_LEVEL feature_level = device->GetFeatureLevel();
	u32 shader_compile_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS;
	
#if DEBUGMODE
	shader_compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	shader_compile_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	Dx11_blob* shader_blob = 0;
	Dx11_blob* error_blob = 0;
	HRESULT hr = D3DCompile(
		source_shaders_file.data,
		source_shaders_file.size,
		0, defines,0,
		entrypoint_name, target_profile,
		shader_compile_flags, 0,
		&shader_blob, &error_blob
	);
	if(hr != S_OK){
		OutputDebugStringA((char*)error_blob->GetBufferPointer());
	}
	ASSERTHR(hr);

	File_data result =  {0};
	size_t shader_blob_size = shader_blob->GetBufferSize();
	ASSERT(shader_blob_size < ~0);
	result.size = (u32)shader_blob_size;
	result.data = arena_push_data(arena, shader_blob->GetBufferPointer(), result.size);
	
	if(shader_blob) shader_blob->Release();
	if(error_blob) error_blob->Release();
	return result;
}

internal void
dx11_create_vs(D3D* dx, File_data vs, Dx11_vertex_shader** result)
{
	HRESULT hr = dx->device->CreateVertexShader(
		vs.data,
		vs.size,
		0,
		result
	);
	ASSERTHR(hr);
}

// internal void
// dx11_create_input_layout(D3D* dx, File_data vs, Dx11_input_layout_desc ied[], s32 ied_count, Dx11_input_layout** result)
// {
// 	HRESULT hr = dx->device->CreateInputLayout(
// 		ied, ied_count, 
// 		vs.data, vs.size, 
// 		result
// 	); 
// 	ASSERTHR(hr);
// }

internal void
dx11_create_ps(D3D* dx, File_data ps, Dx11_pixel_shader** result)
{
	dx->device->CreatePixelShader(
		ps.data,
  		ps.size,
  		0,
		result
	);
}
// CONSTANT BUFFER
internal void
dx11_create_constant_buffer(
	D3D* dx, D3D_constant_buffer* result, 
	u32 buffer_size, Shader_constant_buffer_register_index register_index, 
	void* initialize)
{
	HRESULT hr = 0;
	D3D11_BUFFER_DESC desc = {0};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = buffer_size;
	// desc.ByteWidth = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*16 +16;
	if(initialize)
	{
		D3D11_SUBRESOURCE_DATA init_data = {0};
		init_data.pSysMem = initialize;
		hr = dx->device->CreateBuffer(&desc, &init_data, &result->buffer);
	}else{
		hr = dx->device->CreateBuffer(&desc, 0, &result->buffer);
	}
	ASSERTHR(hr);
	result->size = buffer_size;
	result->register_index = register_index;
}


internal void
dx11_create_sampler(D3D* dx, Dx11_sampler_state** result)
{
	Dx11_sampler_desc desc = {0};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	// D3D11_TEXTURE_ADDRESS_WRAP
	// D3D11_TEXTURE_ADDRESS_BORDER
	// D3D11_TEXTURE_ADDRESS_CLAMP
	#if DEBUGMODE
		D3D11_TEXTURE_ADDRESS_MODE texture_address_mode =  D3D11_TEXTURE_ADDRESS_WRAP;
	#else
		D3D11_TEXTURE_ADDRESS_MODE texture_address_mode = D3D11_TEXTURE_ADDRESS_WRAP;
	#endif
	desc.AddressU = texture_address_mode;
	desc.AddressV = texture_address_mode;
	desc.AddressW = texture_address_mode;
	desc.BorderColor[0] = 1.0f;
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 1.0f;
	desc.BorderColor[3] = 1.0f;
	HRESULT hr = dx->device->CreateSamplerState(&desc, result);
	ASSERTHR(hr);
}
// fill modes: D3D11_FILL_WIREFRAME / D3D11_FILL_SOLID ;  cull modes: D3D11_CULL_BACK | D3D11_CULL_FRONT | D3D11_CULL_NONE
internal void
dx11_create_rasterizer_state(D3D* dx, Dx11_rasterizer_state** result, D3D11_FILL_MODE fill_mode, D3D11_CULL_MODE cull_mode)
{
	HRESULT hr = {0};
	Dx11_rasterizer_desc desc = {0};
	desc.FillMode = fill_mode;
	desc.CullMode = cull_mode;
	hr = dx->device->CreateRasterizerState(&desc, result);
	ASSERTHR(hr);
}

internal bool
dx11_create_blend_state(D3D* dx, Dx11_blend_state** result, bool enable_alpha_blending)
{
	HRESULT hr;
	D3D11_RENDER_TARGET_BLEND_DESC d  = {0};
	d.BlendEnable = enable_alpha_blending;
	d.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	d.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	d.BlendOp = D3D11_BLEND_OP_ADD;
	d.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	d.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	d.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	d.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;    
	D3D11_BLEND_DESC desc = {0};
	desc.RenderTarget[0] = d;
	desc.RenderTarget[1] = d;
	desc.RenderTarget[2] = d;
	desc.RenderTarget[3] = d;
	desc.RenderTarget[4] = d;
	desc.RenderTarget[5] = d;
	desc.RenderTarget[6] = d;
	desc.RenderTarget[7] = d;
	hr = dx->device->CreateBlendState(&desc, result); //TODO: AAAAAAAAAA
	return SUCCEEDED(hr);
}


internal void 
dx11_create_depth_stencil_state(D3D* dx, Dx11_depth_stencil_state** result, bool is_enabled)
{
	HRESULT hr = {0};
	D3D11_DEPTH_STENCIL_DESC desc = {0};
	desc.DepthEnable = is_enabled; 
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	// D3D11_COMPARISON_GREATER/D3D11_COMPARISON_LESS/D3D11_COMPARISON_EQUAL
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = false; // define FrontFace and BackFace if i want to enable
	// desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
	// desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
	// to enable must set this 2
	// desc.FrontFace = ... 
	// desc.BackFace = ...
	hr = dx->device->CreateDepthStencilState(&desc, result);
	ASSERTHR(hr);
}


internal void
dx11_create_depth_stencil_view(D3D* dx, Dx11_depth_stencil_view** result, u32 width, u32 height)
{
    
	// CREATE DEPTH STENCIL TEXTURE
	ID3D11Texture2D* depth_stencil_texture = 0;
	D3D11_TEXTURE2D_DESC dd = {0};
	dd.Width = width;
	dd.Height = height;
	dd.MipLevels = 1; // 1 for a multisampled texture; or 0 to generate a full set of subtextures.
	dd.ArraySize = 1;
	dd.Format = DXGI_FORMAT_D32_FLOAT;
	dd.SampleDesc = { 1, 0 };
	dd.Usage = D3D11_USAGE_DEFAULT;
	dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dx->device->CreateTexture2D(&dd, 0, &depth_stencil_texture);

	D3D11_DEPTH_STENCIL_VIEW_DESC ddsv = {0};
	ddsv.Format = DXGI_FORMAT_D32_FLOAT;
	ddsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	ddsv.Texture2D.MipSlice = 0;

	dx->device->CreateDepthStencilView(depth_stencil_texture, &ddsv, result);
	depth_stencil_texture->Release();
}

internal ID3D11Texture2D*
dx11_create_texture2d(D3D* dx, Surface* texture)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC desc = {0};
	desc.Width = texture->width;
	desc.Height = texture->height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	
	Dx11_subresource_data init_data = {0};
	init_data.pSysMem = texture->data;
	init_data.SysMemPitch = texture->width * sizeof(u32);


	ID3D11Texture2D* texture2D;
	hr = dx->device->CreateTexture2D(&desc, &init_data, &texture2D);
	ASSERTHR(hr);

	return texture2D;

}


internal void
dx11_create_texture_view(D3D* dx, ID3D11Resource* resource, ID3D11ShaderResourceView** texture_view)
{

	// D3D11_SHADER_RESOURCE_VIEW_DESC srd = {0};
	// srd.Format = desc.Format;
	// srd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	// srd.Texture2D.MostDetailedMip = 0;
	// srd.Texture2D.MipLevels = 1;

	// pDesc to 0 to create a view that accesses the entire resource (using the format the resource was created with)
	HRESULT hr = dx->device->CreateShaderResourceView(resource, 0, texture_view);//TODO: this had the srd
	ASSERTHR(hr);
}

internal void
create_screen_render_target_view(D3D* dx, Dx11_render_target_view** result)
{
	HRESULT hr = {0};
	ID3D11Texture2D* back_buffer = {0};
	// hr = dx->swap_chain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&back_buffer);
	hr = dx->swap_chain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&back_buffer);
	ASSERTHR(hr);

	hr = dx->device->CreateRenderTargetView(back_buffer, 0, result);
	ASSERTHR(hr);
	back_buffer->Release();
}
internal void
dx11_set_viewport(D3D* dx, s32 posx, s32 posy, u32 width, u32 height)
{
	Dx11_viewport vp = {0};
	vp.TopLeftX = (f32)posx;
	vp.TopLeftY = (f32)posy;
	vp.Width = (f32)width;
	vp.Height = (f32)height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	dx->viewport = vp;
	dx->context->RSSetViewports(1, &vp);
}
// BINDING FUNCTIONS
internal void
dx11_bind_vs(D3D* dx, Dx11_vertex_shader* vertex_shader)
{
	// BINDING SHADERS AND LAYOUT
	dx->context->VSSetShader(vertex_shader, 0, 0);
}
// internal void
// dx11_bind_ps(D3D* dx, Dx11_pixel_shader* pixel_shader)
// {
// 	dx->context->PSSetShader(pixel_shader, 0, 0);
// }
// internal void
// dx11_bind_input_layout(D3D* dx, Dx11_input_layout* input_layout)
// {
// 	dx->context->IASetInputLayout(input_layout);
// }
internal void
dx11_bind_vertex_buffer(D3D* dx, Dx11_buffer* vertex_buffer, u32 sizeof_vertex)
{
	u32 strides = sizeof_vertex; 
	u32 offsets = 0;
	dx->context->IASetVertexBuffers(
		0,
		1,
		&vertex_buffer,
		&strides,
		&offsets
	);
}

internal void
dx11_bind_sampler(D3D* dx, Dx11_sampler_state** sampler)
{
	dx->context->PSSetSamplers(0, 1, sampler);
}
// internal void
// dx11_bind_blend_state(D3D* dx, ID3D11BlendState* blend_state)
// {
// 	// float blend_factor [4] = {0.0f,0.0f,0.0f,1.0f};
// 	dx->context->OMSetBlendState(blend_state, 0, ~0U);   
// }
internal void
dx11_bind_rasterizer_state(D3D* dx, Dx11_rasterizer_state* rasterizer_state)
{
dx->context->RSSetState(rasterizer_state);
}

internal void
dx11_modify_resource(D3D* dx, Dx11_resource* resource, void* data, u32 size)
{
	D3D11_MAPPED_SUBRESOURCE mapped_resource = {0};
	// disable gpu access 
	HRESULT map_error = dx->context->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	ASSERT(map_error == S_OK);
	// update here
	
	// TODO: i must NEVER read from the pData pointer cuz it will cause huge performance penalties apparently
	// this is an example of code that can trigger the performance penalty:
	// ''' *((int*)MappedResource.pData) = 0;''' 
	// which produces the following assembly code
	// ''' AND DWORD PTR [EAX],0 '''
	// and i am doing exactly that in the copy_mem() function so...
	copy_mem(data, mapped_resource.pData, size);
	// re-enable gpu access

	dx->context->Unmap(resource, 0);
}

internal void
dx11_draw_mesh(D3D* dx, Dx_mesh* mesh){		
	dx->context->IASetPrimitiveTopology( mesh->topology );
	dx11_bind_vertex_buffer(dx, mesh->vertex_buffer, mesh->vertex_size);
	// FINALLY DRAW
	ASSERT(mesh->index_buffer);
	dx->context->IASetIndexBuffer(mesh->index_buffer, DXGI_FORMAT_R16_UINT, 0);
	dx->context->DrawIndexed(mesh->indices_count, 0, 0);
}


internal void
create_vertex_shader(D3D* dx, DXGI_FORMAT* ie_formats_list, u32* ie_formats_sizes, Asset_request* request, File_data compiled_vs, Memory_arena* temp_arena, Vertex_shader* vs)
{
	// COMPILING VS
		// File_data compiled_vs = dx11_get_compiled_shader(request->filename, temp_arena, "vs", VS_PROFILE);
	// CREATING VS

	s32 MAX_IE_SIZE = sizeof(f32)*4;
	
	u32 ie_count = 0;
	#define USE_APPEND_ALIGNED_ELEMENT 1

	#if USE_APPEND_ALIGNED_ELEMENT
	#else
		u32 aligned_byte_offsets [2] = {0};
	#endif
	D3D11_INPUT_ELEMENT_DESC* ied = (D3D11_INPUT_ELEMENT_DESC*)(temp_arena->data+temp_arena->used);
	s32 total_element_size = 0;
	UNTIL(j, ARRAYLEN(request->ied.names))
	{
		s32 current_element_size = ie_formats_sizes[request->ied.formats[j]];
		total_element_size += current_element_size;
		for(s32 semantic_index = 0; current_element_size > 0; semantic_index++)
		{
			ie_count++;
			D3D11_INPUT_ELEMENT_DESC* current_ied = ARENA_PUSH_STRUCT(temp_arena, D3D11_INPUT_ELEMENT_DESC);
			current_ied->SemanticName = request->ied.names[j];
			// this is in case the element is bigger than a float4 (a matrix for example)
			current_ied->SemanticIndex = semantic_index;
			
			current_ied->Format = ie_formats_list[request->ied.formats[j]];

			u32 ie_slot = 0;
			if(request->ied.next_slot_beginning_index && j >= request->ied.next_slot_beginning_index)
			{
				ie_slot = 1;
			}
			
			#if USE_APPEND_ALIGNED_ELEMENT
				current_ied->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			#else
				current_ied->AlignedByteOffset = aligned_byte_offsets[ie_slot];
				aligned_byte_offsets[ie_slot] += MIN(MAX_IE_SIZE,current_element_size); 
			#endif
			current_element_size -= MAX_IE_SIZE;
			
			current_ied->InputSlot = ie_slot;// this is for using secondary buffers (like an instance buffer)
			current_ied->InputSlotClass = (D3D11_INPUT_CLASSIFICATION)ie_slot; // 0 PER_VERTEX_DATA vs 1 PER_INSTANCE_DATA
			current_ied->InstanceDataStepRate = ie_slot; // the amount of instances to draw using the PER_INSTANCE data
		}
	}
	UNTIL(e, ie_count)
	{
		ied[e];
		ASSERT(true);
	}
	HRESULT hr = dx->device->CreateInputLayout(
		ied, ie_count, 
		compiled_vs.data, compiled_vs.size, 
		&vs->input_layout
	); 
	ASSERTHR(hr);	
}