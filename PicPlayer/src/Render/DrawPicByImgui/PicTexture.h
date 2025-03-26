#ifndef PICTEXTURE_H
#define PICTEXTURE_H

#include "GL/glew.h"
#include "../Imgui/imgui.h"
#include "../PicPlayerDataDef.h"
#include "../Utils/Singleton.h"
#include <queue>
#include <unordered_map>

class TexTure2D
{
public:
    uint32_t m_Id;
    uint32_t m_width;
    uint32_t m_height;

public:
    TexTure2D()
        : m_width(0)
        , m_height(0)
    {
        glGenTextures(1, &m_Id);
    }


    void Generate(uint32_t width, uint32_t height, char* rgbaData)
    {
        m_width = width;
        m_height = height;
        // bind
        glBindTexture(GL_TEXTURE_2D, m_Id);
        glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
        // set
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // unbind
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

struct TexInfo
{
    uint32_t m_maxTexWidth;
    uint32_t m_maxTexHeight;
    uint32_t m_texId;
    TexInfo()
        : m_maxTexWidth(1000), m_maxTexHeight(1000), m_texId(0)
    {}
};

class PicTexture : public Singleton<PicTexture>
{
    friend class Singleton<PicTexture>;

public:
    void InitPicTexturePool(int cacheNum);
    void ClearPicTexturePool();

    uint32_t GenTexId();
    uint32_t GetMaxTexWidth(uint32_t texId) const;
    uint32_t GetMaxTexHeight(uint32_t texId) const;
    void ReleaseTexId(uint32_t texId);

    void SetPicTexture(uint32_t texId, const PicShowInfo& picData);
    void DrawPicTexture(uint32_t texId, const ImVec2& drawStart, const ImVec2& drawEnd,const ImVec2& uvStart, const ImVec2& uvEnd);
    void DrawRect(const ImVec2& drawStart, const ImVec2& drawEnd, const ImU32& drawCol);

private:
    PicTexture();
    virtual ~PicTexture();

    void InitTexInfo(TexInfo& curTexInfo);
    void UpdateTex(void* data, size_t dataLen, uint32_t texTureW, uint32_t texTureH);
    void ResizeTex(uint32_t texTureW, uint32_t texTureH, uint32_t newW, uint32_t newH);

private:
    uint32_t m_cacheNum;
    uint32_t m_pboId;
    std::vector<TexInfo> m_texInfoVec;
    std::queue<uint32_t> m_availIds;
    std::unordered_map<std::string, TexTure2D> m_texMap;
};

#endif // PICTEXTURE_H
