/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#ifndef H_GPGPU_GLSLPROGRAM
#define H_GPGPU_GLSLPROGRAM
#include "InternalPrerequisites.h"
#include "ProgramFactory.h"
#include "GLProgram.h"

namespace GPGPU
{
class GLSLProgramFactory: public ProgramFactory
{
public:
    GLSLProgramFactory();
    ~GLSLProgramFactory();
    
    virtual void initialise(HWInfo &info);

    virtual ProgramPtr createProgram(const std::string &language);
};

class GLSLProgram: public GLProgram
{
public:
    GLSLProgram();
    virtual ~GLSLProgram();
    
    /** Set program source code
    */
    virtual void setSource(const std::string &src);
    
    virtual void setEntryPoint(const std::string &src);
    
    /** Compile and initialize the program
    */
    virtual void initialise();
    
    /// Functions to set parameters
    /// 1,2,3,4 floats
    virtual void setParameter(const std::string &name, float x, float y, float z, float w);
    /// 4x4 matrix
    virtual void setParameter(const std::string &name, const float matrix[16]);
    /// Texture (sampler)
    virtual void setParameter(const std::string &name, TexturePtr tex);
    
    /// Get texture units configuration for program from parameters
    std::vector<TexturePtr> &getTextureUnits();
    
    /// Bind program and its parameters
    /// Changes to the parameters while the program is bound must reflect in the rendering API
    void bind();
    /// Unbind program and its parameters
    void unbind();
private:
    bool mIsBound;
    std::string mSourceCode;
    std::vector<TexturePtr> mTextureUnits;
    GLuint mObjFP, mProgram;
    
    enum ParameterClass
    {
        PC_REAL,            /// Float, bool or int
        PC_MATRIX,          /// Matrix
        PC_SAMPLER          /// Sampler
    };
    struct ParameterDefinition
    {
        std::string name;
        ParameterClass type;
        size_t components;
        GLint location;        /// Memory location (GL)
        float curValue[16];    /// Enough for 4x4 matrix
        size_t texunit;        /// Texture unit (in case this is a sampler)
        bool dirty;            /// Parameter was changed since last send to card
    };
    typedef std::map<std::string, ParameterDefinition> ParameterDefinitionMap;
    ParameterDefinitionMap mParameterDefinitions;
    
    ///  Update parameter from memory to card
    void updateParameter(ParameterDefinition &param);
};
typedef SharedPtr<GLSLProgram> GLSLProgramPtr;
};
#endif
