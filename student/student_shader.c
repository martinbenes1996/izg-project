/*!
 * @file
 * @brief This file contains implemenation of phong vertex and fragment shader.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <assert.h>
#include <math.h>

#include <student/gpu.h>
#include <student/student_shader.h>
#include <student/uniforms.h>

/// \addtogroup shader_side Úkoly v shaderech
/// @{

void phong_vertexShader(GPUVertexShaderOutput *const      output,
                        GPUVertexShaderInput const *const input,
                        GPU const                         gpu) {
  /// \todo Naimplementujte vertex shader, který transformuje vstupní vrcholy do
  /// clip-space.<br>
  /// <b>Vstupy:</b><br>
  /// Vstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu ve
  /// world-space (vec3) a v prvním
  /// atributu obsahovat normálu vrcholu ve world-space (vec3).<br>
  /// <b>Výstupy:</b><br>
  /// Výstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu (vec3)
  /// ve world-space a v prvním
  /// atributu obsahovat normálu vrcholu ve world-space (vec3).
  /// Výstupní vrchol obsahuje pozici a normálu vrcholu proto, že chceme počítat
  /// osvětlení ve world-space ve fragment shaderu.<br>
  /// <b>Uniformy:</b><br>
  /// Vertex shader by měl pro transformaci využít uniformní proměnné obsahující
  /// view a projekční matici.
  /// View matici čtěte z uniformní proměnné "viewMatrix" a projekční matici
  /// čtěte z uniformní proměnné "projectionMatrix".
  /// Zachovejte jména uniformních proměnných a pozice vstupních a výstupních
  /// atributů.
  /// Pokud tak neučiníte, akceptační testy selžou.<br>
  /// <br>
  /// Využijte vektorové a maticové funkce.
  /// Nepředávajte si data do shaderu pomocí globálních proměnných.
  /// Pro získání dat atributů použijte příslušné funkce vs_interpret*
  /// definované v souboru program.h.
  /// Pro získání dat uniformních proměnných použijte příslušné funkce
  /// shader_interpretUniform* definované v souboru program.h.
  /// Vrchol v clip-space by měl být zapsán do proměnné gl_Position ve výstupní
  /// struktuře.<br>
  /// <b>Seznam funkcí, které jistě použijete</b>:
  ///  - gpu_getUniformsHandle()
  ///  - getUniformLocation()
  ///  - shader_interpretUniformAsMat4()
  ///  - vs_interpretInputVertexAttributeAsVec3()
  ///  - vs_interpretOutputVertexAttributeAsVec3()

  Uniforms gpuHandle = gpu_getUniformsHandle(gpu);
  // projection matrix
  Mat4 const * const projectionM = shader_interpretUniformAsMat4(gpuHandle, 
                                        getUniformLocation(gpu, "projectionMatrix"));
  // view matrix
  Mat4 const * const viewM = shader_interpretUniformAsMat4(gpuHandle,
                                        getUniformLocation(gpu, "viewMatrix"));
  // input vector
  Vec3 const * const norm = vs_interpretInputVertexAttributeAsVec3(gpu, input, 1);
  Vec3 const * const pos = vs_interpretInputVertexAttributeAsVec3(gpu, input, 0);
  
  // transform
  Mat4 transformationMatrix;
  Vec4 pos_ext;
  copy_Vec3Float_To_Vec4(&pos_ext, pos, (double)1);
  multiply_Mat4_Mat4(&transformationMatrix, projectionM, viewM);
  // transform to the bases
  multiply_Mat4_Vec4(&output->gl_Position, &transformationMatrix, &pos_ext);

  // output
  init_Vec3(vs_interpretOutputVertexAttributeAsVec3(gpu,output,0),
          pos->data[0], pos->data[1], pos->data[2]);
  init_Vec3(vs_interpretOutputVertexAttributeAsVec3(gpu,output,1),
          norm->data[0], norm->data[1], norm->data[2]);
}

float clamp(float v) { return (v<0.f)?0.f:((v>1.f)?1.f:v); }

void phong_fragmentShader(GPUFragmentShaderOutput *const      output,
                          GPUFragmentShaderInput const *const input,
                          GPU const                           gpu) {
  /// \todo Naimplementujte fragment shader, který počítá phongův osvětlovací
  /// model s phongovým stínováním.<br>
  /// <b>Vstup:</b><br>
  /// Vstupní fragment by měl v nultém fragment atributu obsahovat
  /// interpolovanou pozici ve world-space a v prvním
  /// fragment atributu obsahovat interpolovanou normálu ve world-space.<br>
  /// <b>Výstup:</b><br>
  /// Barvu zapište do proměnné color ve výstupní struktuře.<br>
  /// <b>Uniformy:</b><br>
  /// Pozici kamery přečtěte z uniformní proměnné "cameraPosition" a pozici
  /// světla přečtěte z uniformní proměnné "lightPosition".
  /// Zachovejte jména uniformních proměnný.
  /// Pokud tak neučiníte, akceptační testy selžou.<br>
  /// <br>
  /// Dejte si pozor na velikost normálového vektoru, při lineární interpolaci v
  /// rasterizaci může dojít ke zkrácení.
  /// Zapište barvu do proměnné color ve výstupní struktuře.
  /// Shininess faktor nastavte na 40.f
  /// Difuzní barvu materiálu nastavte podle normály povrchu.
  /// V případě, že normála směřuje kolmo vzhůru je difuzní barva čistě bílá.
  /// V případě, že normála směřuje vodorovně nebo dolů je difuzní barva čiště zelená.
  /// Difuzní barvu spočtěte lineární interpolací zelené a bíle barvy pomocí interpolačního parameteru t.
  /// Interpolační parameter t spočtěte z y komponenty normály pomocí t = y*y (samozřejmě s ohledem na negativní čísla).
  /// Spekulární barvu materiálu nastavte na čistou bílou.
  /// Barvu světla nastavte na bílou.
  /// Nepoužívejte ambientní světlo.<br>
  /// <b>Seznam funkcí, které jistě využijete</b>:
  ///  - shader_interpretUniformAsVec3()
  ///  - fs_interpretInputAttributeAsVec3()
  Uniforms gpuHandle = gpu_getUniformsHandle(gpu);
  // projection matrix
  Vec3 const * const cameraPosition = shader_interpretUniformAsVec3(gpuHandle, 
                                        getUniformLocation(gpu, "cameraPosition"));
  // view matrix
  Vec3 const * const lightPosition = shader_interpretUniformAsVec3(gpuHandle,
                                        getUniformLocation(gpu, "lightPosition"));
  // input vector
  Vec3 const * const normal = fs_interpretInputAttributeAsVec3(gpu, input, 1);
  Vec3 const * const position = fs_interpretInputAttributeAsVec3(gpu, input, 0);

  // coefficients
  float rs = 400.f; // reflection
  //float ns = 0.f; // specular
  float rd = 2.f; // diffusion

  // normalize
  Vec3 normNorm; // norm
  normalize_Vec3(&normNorm,normal);
  Vec3 L, normL; // L
  sub_Vec3(&L, lightPosition, position);
  normalize_Vec3(&normL, &L);
  Vec3 Cam, normCam; // cam
  sub_Vec3(&Cam, cameraPosition, position);
  normalize_Vec3(&normCam, &Cam);
  multiply_Vec3_Float(&normCam, &normCam, -1.f);
  Vec3 R, normR; // R
  reflect(&R, &normCam, &normNorm);
  normalize_Vec3(&normR, &R);
  
  // colors
  Vec3 surfC;
  Vec3 lightC;
  init_Vec3(&surfC, 0.f, 1.f, 0.f); // green surface
  if(normNorm.data[1] > 0) init_Vec3(&surfC, clamp((float)pow(normNorm.data[1],2)), 1.f, clamp((float)pow(normNorm.data[1],2)));
  init_Vec3(&lightC, 1.f, 1.f, 1.f); // white light
  
  // characteristics
  float Id = clamp(dot_Vec3(&normNorm,&normL)*rd);
  float Is = clamp( (float)pow(dot_Vec3(&normL,&normCam),rs) );
  if(dot_Vec3(&normL,&normNorm) < 0) Is = 0;
  //float Ia = 0;

  Vec3 result;
  Vec3 tmp;
  //multiply_Vec3_Float(&result,&surfC,Id);
  init_Vec3(&result, clamp(surfC.data[0]*lightC.data[0]*Id), 
                     clamp(surfC.data[1]*lightC.data[1]*Id), 
                     clamp(surfC.data[2]*lightC.data[2]*Id));
  multiply_Vec3_Float(&tmp,&lightC,Is);
  add_Vec3(&result,&result,&tmp);

  for(int i = 0; i < 3; i++) { output->color.data[i] = result.data[i]; }
  output->color.data[3] = 1.f;
  
}

/// @}
