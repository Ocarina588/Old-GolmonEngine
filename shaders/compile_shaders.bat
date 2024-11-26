glslc shaders/raytrace.rgen -o shaders/raygen.spv --target-spv=spv1.4
glslc shaders/raytrace.rchit -o shaders/raychit.spv --target-spv=spv1.4
glslc shaders/raytrace.rmiss -o shaders/raymiss.spv --target-spv=spv1.4
REM glslc -fshader-stage=vertex shaders/vertex.glsl -o shaders/vertex.spv --target-spv=spv1.4 
REM glslc -fshader-stage=fragment shaders/fragment.glsl -o shaders/fragment.spv --target-spv=spv1.4 
