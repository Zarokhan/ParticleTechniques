#include "particleshader.h"

ParticleShader::ParticleShader()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_cameraBuffer = 0;
	m_sampleState = 0;
}

ParticleShader::ParticleShader(const ParticleShader &other)
{
}

ParticleShader::~ParticleShader()
{
	SAFE_RELEASE(m_sampleState);
	SAFE_RELEASE(m_matrixBuffer);
	SAFE_RELEASE(m_cameraBuffer);
	SAFE_RELEASE(m_layout);
	SAFE_RELEASE(m_pixelShader);
	SAFE_RELEASE(m_vertexShader);
}

bool ParticleShader::Initialize(ID3D11Device3 * device, HWND hwnd)
{
	bool result;

	result = InitializeShader(device, hwnd, L"Shaders/particleVS.hlsl", L"Shaders/particlePS.hlsl", L"Shaders/particleGS.hlsl");
	if (!result)
		return false;

	return true;
}

bool ParticleShader::Render(Direct3DManager* direct, ParticleSystem* particlesystem, Camera* camera)
{
	bool result;

	result = SetShaderParameters(direct->GetDeviceContext(), camera, direct->GetWorldMatrix(), camera->GetViewMatrix(), direct->GetProjectionMatrix(), particlesystem->GetTexture());
	if (!result)
		return false;

	RenderShader(direct->GetDeviceContext(), particlesystem->GetVertexCount(), particlesystem->GetInstaceCount(), particlesystem->GetIndexCount());
	return true;
}

bool ParticleShader::InitializeShader(ID3D11Device3 * device, HWND hwnd, WCHAR * vsFilename, WCHAR * psFilename, WCHAR * gsFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage = 0;
	ID3D10Blob* vertexShaderBuffer = 0;
	ID3D10Blob* pixelShaderBuffer = 0;
	ID3D10Blob* geometryShaderBuffer = 0;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Compile Vertex Shader
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "VS_main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMsg(errorMessage, hwnd, vsFilename);
		else
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		return false;
	}
	// Compile Pixel Shader
	result = D3DCompileFromFile(psFilename, NULL, NULL, "PS_main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMsg(errorMessage, hwnd, psFilename);
		else
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		return false;
	}
	// Compile Geometry Shader
	result = D3DCompileFromFile(gsFilename, NULL, NULL, "GS_main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &geometryShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMsg(errorMessage, hwnd, psFilename);
		else
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		return false;
	}

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	result = device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), NULL, &m_geometryShader);
	if (FAILED(result))
		return false;

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "COLOR";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "TEXCOORD";
	polygonLayout[3].SemanticIndex = 1;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 1;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	polygonLayout[3].InstanceDataStepRate = 1;

	polygonLayout[4].SemanticName = "COLOR";
	polygonLayout[4].SemanticIndex = 1;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[4].InputSlot = 1;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	polygonLayout[4].InstanceDataStepRate = 1;

	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	SAFE_RELEASE(vertexShaderBuffer);
	SAFE_RELEASE(pixelShaderBuffer);

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
		return false;

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	if (FAILED(result))
		return false;

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
		return false;

	return true;
}

void ParticleShader::OutputShaderErrorMsg(ID3D10Blob * errorMsg, HWND hwnd, WCHAR * shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMsg->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMsg->GetBufferSize();

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
	SAFE_RELEASE(errorMsg);

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}

bool ParticleShader::SetShaderParameters(ID3D11DeviceContext * deviceContext, Camera * camera, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView * texture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	// Map matrix buffer
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);
	
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);

	// Map camera buffer
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	CameraBufferType* dataPtr2 = (CameraBufferType*)mappedResource.pData;

	dataPtr2->cameraPosition = camera->GetPosition();
	deviceContext->Unmap(m_cameraBuffer, 0);

	deviceContext->GSSetConstantBuffers(0, 1, &m_matrixBuffer);
	deviceContext->GSSetConstantBuffers(1, 1, &m_cameraBuffer);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &texture);
	//deviceContext->OMSetRenderTargetsAndUnorderedAccessViews()

	return true;
}

void ParticleShader::RenderShader(ID3D11DeviceContext * deviceContext, int vertexCount, int instanceCount, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->GSSetShader(m_geometryShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	//deviceContext->DrawIndexed(indexCount, 0, 0);
	deviceContext->DrawInstanced(vertexCount, instanceCount, 0, 0);
}
