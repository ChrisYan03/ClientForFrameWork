# PicMatch 人脸识别数据目录说明

人脸识别（OpenCV haarcascade / DNN / 年龄模型）从 **组件 bin 目录** 加载，不再从主程序 `.app/Contents/MacOS` 加载。

## 正确位置（Mac / Windows 一致）

将以下目录放在 **Component/PicMatch/bin** 下（与 `libPicMatchComponent.dylib` 同级）：

- **data/haarcascades/**  
  - `haarcascade_frontalface_default.xml`  
  - `haarcascade_profileface.xml`  
- **face_detector/**（可选，DNN 模型）  
  - `opencv_face_detector.pbtxt` + `opencv_face_detector_uint8.pb`  
  - 或 `deploy.prototxt` + `res10_300x300_ssd_iter_140000.caffemodel`  
- **age_model/** 或 **models/**（可选，年龄估计）  
  - `age_deploy.prototxt` + `age_net.caffemodel`  

即：`Component/PicMatch/bin/data/`、`Component/PicMatch/bin/face_detector/` 等。

## Mac 上之前放在 .app/Contents/MacOS 的情况

若你曾把上述数据放在  
`ClientForFrame.app/Contents/MacOS/data`、`.../face_detector` 等下，有两种做法：

1. **手动迁移**  
   把 `MacOS/data`、`MacOS/face_detector`、`MacOS/age_model`、`MacOS/models` 整目录复制到  
   `target/Mac/bin/Release/Component/PicMatch/bin/` 下，保持子目录名不变。

2. **用构建自动同步**  
   主程序 CMake 已增加 POST_BUILD：每次构建 ClientForFrame 后，会把  
   `.app/Contents/MacOS` 下的 `data`、`face_detector`、`age_model`、`models` 复制到  
   `Component/PicMatch/bin/`。你只需继续把数据放在 MacOS 下，构建一次后组件即可从新路径加载。

路径逻辑见：`PicMatchWidget.cpp` 中 `InitFaceRecognition(componentBinPath())`，以及  
`PicRecognitionObj.cpp` 中 `init(const char* data_path)` 使用的相对路径。
