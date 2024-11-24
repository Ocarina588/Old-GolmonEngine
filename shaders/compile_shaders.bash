glslc raytrace.rgen -o raygen.spv --target-spv=spv1.4
glslc raytrace.rchit -o raychit.spv --target-spv=spv1.4
glslc raytrace.rmiss -o raymiss.spv --target-spv=spv1.4
