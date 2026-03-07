# 组件加载配置

通过 components.json 决定是否加载各业务组件及其桌面图标；未配置或未列出的组件不加载，主界面为空白。

格式示例（加载 PicMatch 时）：
{
  "components": ["PicMatch"]
}

- 当 "components" 中包含 "PicMatch" 或 "picmatch"：加载图像人脸识别组件，桌面显示该图标，点击可打开对应页面。
- 当配置文件不存在、或 "components" 为空 []、或不包含上述 ID：不注册任何组件，桌面无图标，主界面为空白。

配置查找路径（任一生效即可）：
  1) <exe 所在目录>/config/components.json
  2) <exe 所在目录>/../config/components.json

部署时请将 config 目录与 components.json 放在上述路径之一。
