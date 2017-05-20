#include "SuperShader.h"

SuperShader::SuperShader()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
	m_cameraBuffer = 0;
	m_lightBuffer = 0;
	m_objectBuffer = 0;
	m_reflectionBuffer = 0;
}

SuperShader::SuperShader(const SuperShader& other)
{
}

SuperShader::~SuperShader()
{
	SAFE_RELEASE(m_lightBuffer);
	SAFE_RELEASE(m_sampleState);
	SAFE_RELEASE(m_matrixBuffer);
	SAFE_RELEASE(m_layout);
	SAFE_RELEASE(m_pixelShader);
	SAFE_RELEASE(m_vertexShader);
	SAFE_RELEASE(m_cameraBuffer);
	SAFE_RELEASE(m_objectBuffer);
	SAFE_RELEASE(m_reflectionBuffer);
	SAFE_RELEASE(m_lightVSBuffer);
}

bool SuperShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result = InitializeShader(device, hwnd, L"Shaders/supershaderVS.hlsl", L"Shaders/supershaderPS.hlsl");
	if (!result)
		return false;

	return true;
}

bool SuperShader::Render(Direct3DManager& direct3D, Camera& camera, const Light& light, Model& model, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix, ID3D11ShaderResourceView* depthMapTexture, XMFLOAT3 lightPosition, XMMATRIX reflectionMatrix, ID3D11ShaderResourceView* reflectionTexture)
{
	bool result = SetShaderParameters(direct3D, camera, light, model, lightViewMatrix, lightProjectionMatrix, depthMapTexture, lightPosition, reflectionMatrix, reflectionTexture);
	if (!result)
		return false;

	RenderShader(direct3D.GetDeviceContext(), model.GetIndexCount());
	return true;
}

bool SuperShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC objectBufferDesc;
	D3D11_BUFFER_DESC reflectionBufferDesc;
	D3D11_BUFFER_DESC lightVSBufferDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "VS_main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "PS_main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Handles 3xfloat
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;					// Handles 2xfloat
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Handles 3xfloat as POSITION
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "TANGENT";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "BINORMAL";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	SAFE_RELEASE(vertexShaderBuffer);
	SAFE_RELEASE(pixelShaderBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;		// D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_FILTER_ANISOTROPIC
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;	// D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_CLAMP
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
		return false;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
		return false;

	// Setup the description fo the camera dynamic constant buffer that is in the vs
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the camera constant buffer pointer so we can access the vs constant buffer from within this class
	result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	if (FAILED(result))
		return false;

	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
		return false;

	objectBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	objectBufferDesc.ByteWidth = sizeof(ObjectBufferType);
	objectBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	objectBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	objectBufferDesc.MiscFlags = 0;
	objectBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&objectBufferDesc, NULL, &m_objectBuffer);
	if (FAILED(result))
		return false;

	reflectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	reflectionBufferDesc.ByteWidth = sizeof(ReflectionBufferType);
	reflectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	reflectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	reflectionBufferDesc.MiscFlags = 0;
	reflectionBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&reflectionBufferDesc, NULL, &m_reflectionBuffer);
	if (FAILED(result))
		return false;

	lightVSBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightVSBufferDesc.ByteWidth = sizeof(LightBufferVSType);
	lightVSBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightVSBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightVSBufferDesc.MiscFlags = 0;
	lightVSBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightVSBufferDesc, NULL, &m_lightVSBuffer);
	if (FAILED(result))
		return false;

	return true;
}

void SuperShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	SAFE_RELEASE(errorMessage);

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}

bool SuperShader::SetShaderParameters(Direct3DManager& direct3D, Camera& camera, const Light& light, Model& model, 
	XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix, 
	ID3D11ShaderResourceView* depthMapTexture, XMFLOAT3 lightPosition, XMMATRIX reflectionMatrix, ID3D11ShaderResourceView* reflectionTexture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr = 0;
	LightBufferType* dataPtr2 = 0;
	CameraBufferType* dataPtr3 = 0;
	ObjectBufferType* dataPtr4 = 0;
	LightBufferVSType* dataPtr6 = 0;
	ReflectionBufferType* dataPtr5 = 0;


	result = direct3D.GetDeviceContext()->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = XMMatrixTranspose(model.GetWorldMatrix());
	dataPtr->view = XMMatrixTranspose(camera.GetViewMatrix());
	dataPtr->projection = XMMatrixTranspose(direct3D.GetProjectionMatrix());
	dataPtr->lightProjection = XMMatrixTranspose(lightProjectionMatrix);
	dataPtr->lightView = XMMatrixTranspose(lightViewMatrix);

	direct3D.GetDeviceContext()->Unmap(m_matrixBuffer, 0);
	direct3D.GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	result = direct3D.GetDeviceContext()->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr3 = (CameraBufferType*)mappedResource.pData;
	dataPtr3->cameraPosition = camera.GetPosition();
	dataPtr3->padding = 13.37f;

	direct3D.GetDeviceContext()->Unmap(m_cameraBuffer, 0);
	direct3D.GetDeviceContext()->VSSetConstantBuffers(1, 1, &m_cameraBuffer);

	ID3D11ShaderResourceView* textures[] = { model.GetTexture(), model.GetNormalTexture() };
	direct3D.GetDeviceContext()->PSSetShaderResources(0, 2, textures);

	result = direct3D.GetDeviceContext()->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr2 = (LightBufferType*)mappedResource.pData;
	dataPtr2->ambientColor = light.GetAmbientColor();
	dataPtr2->diffuseColor = light.GetDiffuseColor();
	dataPtr2->lightDirection = light.GetDirection();
	dataPtr2->specularColor = light.GetSpecularColor();
	dataPtr2->specularPower = light.GetSpecularPower();

	direct3D.GetDeviceContext()->Unmap(m_lightBuffer, 0);
	direct3D.GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_lightBuffer);

	result = direct3D.GetDeviceContext()->Map(m_objectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr4 = (ObjectBufferType*)mappedResource.pData;
	dataPtr4->textureScale = model.GetTextureScale();
	dataPtr4->padding = XMFLOAT3();

	direct3D.GetDeviceContext()->Unmap(m_objectBuffer, 0);
	direct3D.GetDeviceContext()->VSSetConstantBuffers(2, 1, &m_objectBuffer);

	result = direct3D.GetDeviceContext()->Map(m_lightVSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr6 = (LightBufferVSType*)mappedResource.pData;
	dataPtr6->lightPosition = lightPosition;
	dataPtr6->padding = 0.0f;

	direct3D.GetDeviceContext()->Unmap(m_lightVSBuffer, 0);
	direct3D.GetDeviceContext()->VSSetConstantBuffers(4, 1, &m_lightVSBuffer);

	direct3D.GetDeviceContext()->PSSetShaderResources(3, 1, &depthMapTexture);

	if (reflectionTexture == 0)
		return true;

	result = direct3D.GetDeviceContext()->Map(m_reflectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr5 = (ReflectionBufferType*)mappedResource.pData;
	dataPtr5->reflectionMatrix = XMMatrixTranspose(reflectionMatrix);

	direct3D.GetDeviceContext()->Unmap(m_reflectionBuffer, 0);
	direct3D.GetDeviceContext()->VSSetConstantBuffers(3, 1, &m_reflectionBuffer);
	direct3D.GetDeviceContext()->PSSetShaderResources(2, 1, &reflectionTexture);

	return true;
}

void SuperShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->GSSetShader(NULL, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}