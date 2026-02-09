#include "PicTexture.h"

PicTexture::PicTexture()
    : m_cacheNum(10)
    , m_pboId(0)
{

}

PicTexture::~PicTexture()
{

}

void PicTexture::InitPicTexturePool(int cacheNum)
{
    m_cacheNum = cacheNum;
    m_texInfoVec = std::vector<TexInfo>(cacheNum);
    glGenBuffers(1, &m_pboId);
    for (int i = 0; i < cacheNum; ++i) {
        auto& curTexInfo = m_texInfoVec.at(i);
        InitTexInfo(curTexInfo);
        m_availIds.push(curTexInfo.m_texId);
    }
}

void PicTexture::InitTexInfo(TexInfo& curTexInfo)
{
    glGenTextures(1, &curTexInfo.m_texId);
    glBindTexture(GL_TEXTURE_2D, (GLuint)curTexInfo.m_texId);
    glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, curTexInfo.m_maxTexWidth, curTexInfo.m_maxTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // set
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // unbind
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PicTexture::ClearPicTexturePool()
{
    for (int i = 0; i < m_cacheNum; ++i) {
        auto& curTexInfo = m_texInfoVec.at(i);
        glDeleteTextures(1, &curTexInfo.m_texId);
    }
    m_texInfoVec.clear();
    while (!m_availIds.empty()) {
        m_availIds.pop();
    }
    glDeleteBuffers(1, &m_pboId);
    m_pboId = 0;
    // for (auto texMark : m_texMap) {
    //     glDeleteTextures(1, (GLuint*)&texMark.second);
    // }
    // m_texMap.clear();
}

uint32_t PicTexture::GenTexId()
{
    uint32_t texid = 0;
    if (!m_availIds.empty()) {
        texid = m_availIds.front();
        m_availIds.pop();
    }
    return texid;
}

uint32_t PicTexture::GetMaxTexWidth(uint32_t texId) const
{
    auto iter = std::find_if(m_texInfoVec.begin(), m_texInfoVec.end(), [texId](const TexInfo& texData){
        return texData.m_texId == texId;
    });
    if (iter != m_texInfoVec.end()) {
        return iter->m_maxTexWidth;
    }
    return 1000;
}

uint32_t PicTexture::GetMaxTexHeight(uint32_t texId) const
{
    auto iter = std::find_if(m_texInfoVec.begin(), m_texInfoVec.end(), [texId](const TexInfo& texData){
        return texData.m_texId == texId;
    });
    if (iter != m_texInfoVec.end()) {
        return iter->m_maxTexHeight;
    }
    return 1000;
}

void PicTexture::ReleaseTexId(uint32_t texId)
{
    m_availIds.push(texId);
}

void PicTexture::SetPicTexture(uint32_t texId, std::shared_ptr<PicShowInfo> picData)
{
    auto iter = std::find_if(m_texInfoVec.begin(), m_texInfoVec.end(), [texId](const TexInfo& texData){
        return texData.m_texId == texId;
    });
    if (iter != m_texInfoVec.end()) {
        glBindTexture(GL_TEXTURE_2D, (GLuint)texId);
        if (picData->picWidth > iter->m_maxTexWidth || picData->picHeight > iter->m_maxTexHeight) {
            ResizeTex(iter->m_maxTexWidth, iter->m_maxTexHeight, picData->picWidth, picData->picHeight);
            iter->m_maxTexWidth = picData->picWidth;
            iter->m_maxTexHeight = picData->picHeight;
        }
        UpdateTex((void *)picData->imageRgbaData, picData->imageRgbaLen, picData->picWidth, picData->picHeight);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void PicTexture::DrawPicTexture(uint32_t texId, const ImVec2& drawStart, const ImVec2& drawEnd,const ImVec2& uvStart, const ImVec2& uvEnd)
{
    ImGui::GetWindowDrawList()->AddImage((ImTextureID)texId, drawStart, drawEnd, uvStart, uvEnd);
}

void PicTexture::DrawRect(const ImVec2& drawStart, const ImVec2& drawEnd, const ImU32& drawCol)
{
    ImGui::GetWindowDrawList()->AddRect(drawStart, drawEnd, drawCol);
}

void PicTexture::UpdateTex(void* data, size_t dataLen, uint32_t texTureW, uint32_t texTureH)
{
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint)m_pboId);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, dataLen, NULL, GL_STREAM_DRAW);
    // map the pbo to client memory
    GLubyte* bytePtr = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, dataLen, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (bytePtr) {
        std::memcpy(bytePtr, data, dataLen);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texTureW, texTureH, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// 重新定义纹理大小并拷贝（整图数据基本上不涉及）
void PicTexture::ResizeTex(uint32_t texTureW, uint32_t texTureH, uint32_t newW, uint32_t newH)
{
    // 绑定pbo
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pboId);
    glBufferData(GL_PIXEL_PACK_BUFFER, texTureW * texTureH * 4, nullptr, GL_STATIC_READ);

    // read old tex
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // resize tex size
    glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, newW, newH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // update tex
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newW, newH, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // 清理
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

