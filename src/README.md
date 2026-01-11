# ICG_FINAL_PROJECT

## Project 概覽

- **動畫**：從 FBX 檔案讀取並播放骨骼動畫。
- **運鏡**：透過腳本控制時間軸以及攝影機的角度、演員的動作和事件觸發。
- **Geometry Shader**：用於實作Explode、下雨、Burning、Motion Blur和Shockwave等特效。
- **粒子系統**：自定義的雨天、energy beam和Shockwave rings。

## 目錄結構

```
src/
├── main_animated.cpp     
├── cinematic_director.cpp  # 電影導演系統
├── animated_model.cpp    
├── static_model.cpp      
├── shader.cpp            
├── rain.cpp                # 雨滴粒子系統
├── energy_beam.cpp         # Energy beam系統
├── shockwave_rings.cpp     # Shockwave rings系統
├── stb_image.cpp         
├── header/               
├── shaders/              
├── asset/                
│   ├── obj/              
│   ├── texture/          
│   └── *.fbx             
└── CMakeLists.txt        
```

### 核心原始碼

* **`main_animated.cpp`**

  * 程式主入口，包含 `main()`、`setup()`、`update()` 和 `render()` 。
  * 管理render loop和特效開關（如 `enableExplode`、`enableShockwave`、`enableRain`）。
  * 處理keycallback。
* **`cinematic_director.cpp` / `cinematic_director.h`**

  * **時間軸管理**：追蹤global time `m_GlobalTime`。
  * **攝影機關鍵影格**：在預設鏡頭（廣角、中景、特寫、跟隨、環繞鏡頭等）之間移動攝影機。
  * **場面調度**：控制角色狀態（行走 → 撞擊 → 飛行 → 翻滾）和車輛位置。
  * **程序化動畫**：在特定時間動態旋轉角色的頭部以注視攝影機。
  * **碰撞偵測**：監控車輛與角色之間的距離以觸發撞擊事件。
  * **物理模擬**：控制角色被撞飛後的拋物線運動和地面翻滾。
* **`static_model.cpp` / `static_model.h`**

  * 使用 Assimp 處理 OBJ 檔案的載入。
  * 管理多材質和多紋理的模型渲染。

### 特效系統

* **`rain.cpp` / `rain.h`**

  * 雨天粒子系統，產生隨機分布的雨滴。
  * 配合Geometry Shader將點擴展為線段，模擬下落的雨滴。
* **`shockwave_rings.cpp` / `shockwave_rings.h`**

  * Shockwave rings系統，在碰撞發生時渲染從中心向外擴散的環形波紋。
  * 使用Geometry Shader生成環形幾何體。
* **`energy_beam.cpp` / `energy_beam.h`**

  * Energy beam系統，渲染從碰撞點爆發出的多方向光線。
  * 使用隨機顏色混合（紅、橘、黃）增加視覺效果。

### Geometry Shader特效

| 著色器名稱           | 檔案                          | 說明                       |
| -------------------- | ----------------------------- | -------------------------- |
| `animated_explode` | `.vert` `.geom` `.frag` | 將三角形沿法線方向向外推移 |
| `burning`          | `.vert` `.geom` `.frag` | 為車輛添加火焰動態效果     |
| `motion_blur`      | `.vert` `.geom` `.frag` | 為移動的車輛生成殘影       |
| `rain`             | `.vert` `.geom` `.frag` | 將點擴展為帶有透明度的線段 |
| `shockwave`        | `.vert` `.geom` `.frag` | 生成向外擴散的環形幾何體   |
| `energy_beam`      | `.vert` `.geom` `.frag` | 從中心點向外發射光線       |

## 運鏡腳本說明

1. **開場 (0-5秒)**

   - 角色向前行走
   - 攝影機從廣角過渡到特寫
2. **轉頭 (5-6秒)**

   - 角色的頭部轉向攝影機方向
3. **車輛逼近 (6秒+)**

   - 車輛駛入場景
   - 移動時產生Motion blur效果
4. **撞擊**

   - 導演系統偵測到碰撞
   - **視覺效果**：Shockwave rings從撞擊點擴散、Energy beam爆發
   - **物理效果**：角色被撞飛到空中
5. **後續**

   - 角色落地後翻滾
   - **爆炸**：翻滾結束後，角色爆炸
   - **燃燒**：車輛開始燃燒

## 控制方式

### 播放控制

| 按鍵    | 功能         |
| ------- | ------------ |
| `C`   | 開始電影動畫 |
| `ESC` | 退出程式     |

### 攝影機控制（手動模式）

| 按鍵                  | 功能              |
| --------------------- | ----------------- |
| `A` / `D`         | 環繞左/右旋轉     |
| `W` / `S`         | 縮放（拉近/拉遠） |
| `Space` / `Shift` | 環繞上/下旋轉     |

### 特效控制

| 按鍵  | 功能              |
| ----- | ----------------- |
| `R` | 切換下雨效果開/關 |

## Dependencies

| 函式庫              | 用途                           |
| ------------------- | ------------------------------ |
| **GLFW**      | 視窗管理和輸入處理             |
| **GLAD**      | OpenGL 函式載入器              |
| **GLM**       | 數學運算（向量、矩陣、四元數） |
| **Assimp**    | 3D 模型載入（FBX、OBJ）        |
| **stb_image** | 紋理圖像載入                   |

## 如何建置

### 前置需求

- C++17 相容的編譯器
- CMake 3.10 或更高版本
- 已安裝上述依賴庫

### Linux / macOS

拿我們的src取代HW3_animated project的src

cmake CMakeList.txt

make

cd src && ./ICG_2024_HW3_Animated

```

```

### ce

* **`3D Model`**
  * [City Model](https://sketchfab.com/3d-models/city-scene-c00b3b8c71594890a2a3c46c20a30b5c?fbclid=IwY2xjawO1IpxleHRuA2FlbQIxMQBzcnRjBmFwcF9pZAEwAAEeF5DnXdz6mMQFFYdaNNSXAR99tzHrwbwcJ_cS432nvdIxO_59ZAoisrf0iIc_aem_n2JQPY-o9oRhIrD4xSRhDA)
  * [Car Model](https://free3d.com/3d-model/low-poly-car-40967.html?dd_referrer=)
  * [People Model](l.messenger.com/l.php?u=https%3A%2F%2Ffree3d.com%2F3d-model%2Feric-rigged-001-771956.html&h=AT2urcssUAvharqdi5duyCBnL7EobTtl7QUaQF8aCN96X0K0MiZhPhKfj8AXU9kIICeX5yLp-_VSj-waSGQYeCYU6-IO-HtWU1RpsvIoN3e2_RwZRERPgc-aDV2hKq0)
  * [Add Animation](https://www.mixamo.com/?fbclid=IwY2xjawO1teNleHRuA2FlbQIxMQBzcnRjBmFwcF9pZAEwAAEegHXi5Lu9KYbgeM2DhYCSxmR_vKpKGFtOBG8ge7FKYs3e3-3QcQo5sgoTxbc_aem_EcygPJkG7OpMkaQmvUIiaA#/)
* **`Cubemap`**
  * [Cubemap Texture](https://polyhaven.com/hdris)
  * [HDRI to CubeMap](https://matheowis.github.io/HDRI-to-CubeMap/?fbclid=IwY2xjawO0bGhleHRuA2FlbQIxMQBzcnRjBmFwcF9pZAEwAAEe1CNLItrMTlzirJUg6GecQm0RalE0fz-j-o_kf6DLOH4ZYahv3et8nvlXZyk_aem_ZbxxrfZEyUpPLtarNHaIZQ)
* **`Shader`**
  * [Explode](http://learnopengl.com/Advanced-OpenGL/Geometry-Shader)
