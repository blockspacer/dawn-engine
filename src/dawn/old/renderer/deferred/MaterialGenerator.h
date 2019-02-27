/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2019 (git@dga.me.uk)
 */
#pragma once

namespace dw {

// Caching, on-the-fly material generator. This is a class that automatically generates and stores
// different permutations of a material, and its shaders. It can be used if you have a material that
// has lots of slightly different variations, like whether to use a specular light, skinning, normal
// mapping and other options. Writing all these out is a tedious job. Of course it is possible to
// always use the material with all features, but that might result in large, slow shader programs.
// This class provides an efficient solution to that.
class DW_API MaterialGenerator {
public:
    // Bitfield used to signify a material permutations
    typedef uint32_t Perm;

    virtual ~MaterialGenerator();

    const Ogre::MaterialPtr& GetMaterial(Perm permutation);

protected:
    // The constructor is protected as this base class should never be constructed as-is. It is
    // meant to be sub classed so that values can be assigned to the various fields controlling
    // material generator, and most importantly, the mImpl field.
    MaterialGenerator();

    const Ogre::GpuProgramPtr& GetVertexShader(Perm permutation);
    const Ogre::GpuProgramPtr& GetFragmentShader(Perm permutation);
    const Ogre::MaterialPtr& GetTemplateMaterial(Perm permutation);

    // Material generators
    virtual Ogre::GpuProgramPtr GenerateVertexShader(Perm permutation) = 0;
    virtual Ogre::GpuProgramPtr GenerateFragmentShader(Perm permutation) = 0;
    virtual Ogre::MaterialPtr GenerateTemplateMaterial(Perm permutation) = 0;

    // Base name of materials generated by this
    Ogre::String mMaterialBaseName;

    // Mask of permutation bits that influence vertex shader choice
    Perm mVsMask;

    // Mask of permutation bits that influence fragment shader choice
    Perm mFsMask;

    // Mask of permutation bits that influence template material choice
    Perm mMatMask;

    typedef Map<Perm, Ogre::GpuProgramPtr> ProgramMap;
    typedef Map<Perm, Ogre::MaterialPtr> MaterialMap;

    ProgramMap mVsMap, mFsMap;
    MaterialMap mTemplateMatMap, mMaterials;
};
}  // namespace dw
