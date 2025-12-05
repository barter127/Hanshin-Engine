
#ifndef RENDER_TEXTURE_CLASS_H
#define RENDER_TEXTURE_CLASS_H

#include <d3d11.h>
#include <directxmath.h>

class RenderTextureClass
{
public:
    RenderTextureClass();
    ~RenderTextureClass();

    bool Initialise(ID3D11Device*, int screenWidth, int screenHeight, float screenDepth, float screenNear, int format);
    HRESULT CopyResource(ID3D11Device* device, ID3D11DeviceContext* devCon, ID3D11ShaderResourceView* resourceView);
    void Shutdown();

    void SetRenderTarget(ID3D11DeviceContext* deviceCon);
    void ClearRenderTarget(ID3D11DeviceContext* deviceCon, float r, float g, float b, float a);
    ID3D11ShaderResourceView* GetShaderResourceView();
    ID3D11Texture2D* GetTexture();

    int GetTextureWidth();
    int GetTextureHeight();

private:
    int m_textureWidth, m_textureHeight;
    ID3D11Texture2D* m_renderTargetTexture;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11ShaderResourceView* m_shaderResourceView;
    ID3D11Texture2D* m_depthStencilBuffer;
    ID3D11DepthStencilView* m_depthStencilView;
    D3D11_VIEWPORT m_viewport;
    DirectX::XMMATRIX m_projectionMatrix;
    DirectX::XMMATRIX m_orthoMatrix;
};

#endif // !RENDER_TEXTURE_CLASS_H