/** This source file is part of
    :: libGPGPU ::
    Abstraction library for general purpose computation on graphics hardware
     W.J. van der Laan 2005
 */
#include "GLSLProgram.h"
#include "GLEngine.h"

namespace GPGPU
{
    GLSLProgramFactory::GLSLProgramFactory()
    {
    }
    GLSLProgramFactory::~GLSLProgramFactory()
    {
    }
    
    void GLSLProgramFactory::initialise(HWInfo &info)
    {

    }

    ProgramPtr GLSLProgramFactory::createProgram(const std::string &language)
    {
        return ProgramPtr(new GLSLProgram());
    }
    
    GLSLProgram::GLSLProgram():
        mIsBound(false),mObjFP(0),mProgram(0)
    {
        
    }
    GLSLProgram::~GLSLProgram()
    {
        glDetachObjectARB(mProgram,mObjFP);
        glDeleteObjectARB(mObjFP);
        glDeleteObjectARB(mProgram);
        GPGPU_IGNOREGLERR();
    }
    
    void GLSLProgram::setSource(const std::string &src)
    {
        mSourceCode = src;
    }
    
    void GLSLProgram::setEntryPoint(const std::string &src)
    {
        if(src != "main")
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Entry point is fixed to main for GLSL");
    }
   
    void GLSLProgram::initialise()
    {
        #define BUFFERSIZE 4096
        GLcharARB infolog[BUFFERSIZE];
        GLsizei length;
        /// Create shader object
        mObjFP = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
        GPGPU_CHECKGLERR("glCreateShaderObjectARB");
        
        /// Upload code for compilation
        const GLcharARB* code = reinterpret_cast<const GLcharARB*>(mSourceCode.data());
        length = mSourceCode.size();
        glShaderSourceARB(mObjFP, 1, &code, &length);
        GPGPU_CHECKGLERR("glShaderSourceARB");
        
        /// Compile shader object
        glCompileShaderARB(mObjFP);
        GPGPU_CHECKGLERR("glCompileShaderARB");
        GLint compiled;
        glGetObjectParameterivARB(mObjFP, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
        if(compiled != GL_TRUE)
        {
            glGetInfoLogARB(mObjFP, BUFFERSIZE, &length, infolog);
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "GLSL compile error: "+std::string(infolog, length));
        }
        
        /// Create program object
        mProgram = glCreateProgramObjectARB();
        GPGPU_CHECKGLERR("glCreateProgramObjectARB");
		
        /// Attach fragment shader
		glAttachObjectARB(mProgram,mObjFP);
        GPGPU_CHECKGLERR("glAttachObjectARB");
	
		glLinkProgramARB(mProgram);
        GPGPU_CHECKGLERR("glLinkProgramARB");
        GLint linked;
        glGetObjectParameterivARB(mProgram, GL_OBJECT_LINK_STATUS_ARB, &linked);
        if(linked!= GL_TRUE)
        {
            glGetInfoLogARB(mProgram, BUFFERSIZE, &length, infolog);
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "GLSL link error: "+std::string(infolog, length));
        }
        
        /// Build parameter map
        int    uniformCount;
        GLint  size;
        char   uniformName[BUFFERSIZE];
        GLenum type;
        size_t texunits = 0;

        // get the number of active uniforms
        glGetObjectParameterivARB(mProgram, GL_OBJECT_ACTIVE_UNIFORMS_ARB,
                &uniformCount);
        
        // Loop over each of the active uniforms, and add them to the reference container
        // only do this for user defined uniforms, ignore built in gl state uniforms
        for (int index = 0; index < uniformCount; index++)
        {
            glGetActiveUniformARB(mProgram, index, BUFFERSIZE, NULL, &size, &type, uniformName);
            
            GLint location = glGetUniformLocationARB(mProgram, uniformName);
            if(location < 0)
                // don't add built in uniforms 
                continue;

            ParameterDefinition def;
            // user defined uniform found, add it to the reference list
            def.name = std::string(uniformName);
            def.location = location;
            def.dirty = false;        /// Parameter initially hasn't changed, let it keep its default
            // decode uniform size and type
            switch (type)
            {
            /// Vector types
            //case GL_INT:
            case GL_FLOAT:
            //case GL_BOOL_ARB:
                def.type = PC_REAL;
                def.components = 1;
                break;
            //case GL_INT_VEC2_ARB:
            case GL_FLOAT_VEC2_ARB:
            //case GL_BOOL_VEC2_ARB:
                def.type = PC_REAL;
                def.components = 2;
                break;
            //case GL_INT_VEC3_ARB:
            case GL_FLOAT_VEC3_ARB:
            //case GL_BOOL_VEC3_ARB:
                def.type = PC_REAL;
                def.components = 3;
                break;
            //case GL_INT_VEC4_ARB:
            case GL_FLOAT_VEC4_ARB:
            //case GL_BOOL_VEC4_ARB:
                def.type = PC_REAL;
                def.components = 4;
                break;
            /// Matrix types
            case GL_FLOAT_MAT2_ARB:                
                def.type = PC_MATRIX;
                def.components = 4;
                break;
            case GL_FLOAT_MAT3_ARB:
                def.type = PC_MATRIX;
                def.components = 9;
                break;
            case GL_FLOAT_MAT4_ARB:
                def.type = PC_MATRIX;
                def.components = 16;
                break;
            /// Sampler types
            case GL_SAMPLER_1D_ARB:
            case GL_SAMPLER_2D_ARB:
            case GL_SAMPLER_3D_ARB:
            case GL_SAMPLER_2D_RECT_ARB:
                def.type = PC_SAMPLER;
                def.components = 1;
                def.texunit = texunits;
                def.dirty = true;        /// Force uniform[texunit] set next time program is bound
                texunits++;
                break;
            default:
                GPGPU_EXCEPT(ERR_INVALIDPARAMS, "Invalid parameter type in GLSL program");
            }// end switch

            /// Put in map
            mParameterDefinitions[def.name] = def;
            /// std::cerr << def.name << " " << def.location << " " << def.type << " " << def.components << std::endl;

        } // end for
        
        /// Resize texture units array to the number of texture units actually used
        mTextureUnits.resize(texunits);
    }
    
    void GLSLProgram::setParameter(const std::string &name, float x, float y, float z, float w)
    {
        ParameterDefinitionMap::iterator i = mParameterDefinitions.find(name);
        if(i == mParameterDefinitions.end())
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "No such (active) parameter "+name);
        
        i->second.dirty = true;
        i->second.curValue[0] = x;
        i->second.curValue[1] = y;
        i->second.curValue[2] = z;
        i->second.curValue[3] = w;
        
        if(mIsBound)
            updateParameter(i->second);
        //GLint loc = glGetUniformLocationARB(mProgram, name.c_str());
        //glUniform4fARB(loc, x, y, z, w);
    }

    void GLSLProgram::setParameter(const std::string &name, const float matrix[16])
    {
        ParameterDefinitionMap::iterator i = mParameterDefinitions.find(name);
        if(i == mParameterDefinitions.end())
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "No such (active) parameter "+name);
        
        i->second.dirty = true;
        memcpy(i->second.curValue, matrix, 16*sizeof(float));
        
        if(mIsBound)
            updateParameter(i->second);
    }

    void GLSLProgram::setParameter(const std::string &name, TexturePtr tex)
    {
        ParameterDefinitionMap::iterator i = mParameterDefinitions.find(name);
        if(i == mParameterDefinitions.end())
            GPGPU_EXCEPT(ERR_INVALIDPARAMS, "No such (active) parameter "+name);
        
        mTextureUnits[i->second.texunit] = tex;
    }
    
    std::vector<TexturePtr> &GLSLProgram::getTextureUnits()
    {
        return mTextureUnits;
    }
    
    void GLSLProgram::updateParameter(ParameterDefinition &param)
    {
        if(!param.dirty)
            /// Updates only needed when a parameter changed, because GLSL remembers the values internally
            return;
        switch(param.type)
        {
        case PC_REAL:
            switch(param.components)
            {
            case 1: glUniform1fvARB(param.location, 1, param.curValue); break;
            case 2: glUniform2fvARB(param.location, 1, param.curValue); break;
            case 3: glUniform3fvARB(param.location, 1, param.curValue); break;
            case 4: glUniform4fvARB(param.location, 1, param.curValue); break;
            }
            GPGPU_CHECKGLERR("glUniformXfvARB");
            break;
        case PC_MATRIX:
            switch(param.components)
            {
            case 2: glUniformMatrix2fvARB(param.location, 1, false, param.curValue); break;
            case 9: glUniformMatrix3fvARB(param.location, 1, false, param.curValue); break;
            case 16: glUniformMatrix4fvARB(param.location, 1, false, param.curValue); break;
            }
            break;
            GPGPU_CHECKGLERR("glUniformMatrixXfvARB");
        case PC_SAMPLER:
            /// Uniform must be texture unit that the texture is assigned to
            glUniform1iARB(param.location, param.texunit);
            GPGPU_CHECKGLERR("glUniform1iARB");
            break;
        }
        param.dirty = false;
    }
    
    /// Bind program and its parameters
    void GLSLProgram::bind()
    {
        GPGPU_ASSERT(!mIsBound, "GLSL program was already bound");
        
		glUseProgramObjectARB(mProgram);
        /// Update parameters that changed since last bind
        for(ParameterDefinitionMap::iterator i=mParameterDefinitions.begin(); i!=mParameterDefinitions.end(); i++)
            updateParameter(i->second);
        mIsBound = true;
    }
    /// Unbind program and its parameters
    void GLSLProgram::unbind()
    {
        GPGPU_ASSERT(mIsBound, "GLSL program was not bound");
        
        glUseProgramObjectARB(0);
        mIsBound = false;
    }
};
