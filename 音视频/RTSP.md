# RTSP

本文档为RTSP详细解析，入门理解详见[链接](./RTSP&RTP&RTCP.md)

## 什么是RTSP

RTSP(Real-Time Stream Protocol)是一种**基于文本的应用层协议**，在语法及一些消息参数等方面，**RTSP协议与HTTP协议类似**。是TCP/IP协议体系中的一个应用层协议, 由哥伦比亚大学, 网景和RealNetworks公司提交的IETF RFC标准.

该协议定义了一对多应用程序如何有效地通过IP网络传送多媒体数据. **RTSP在体系结构上位于RTP和RTCP之上， 通常在TCP连接上进行，但也可以通过UDP或TCP/UDP混合方式进行传输。**

RTSP被用于建立的控制媒体流的传输，它为多媒体服务扮演“网络远程控制”的角色。尽管有时可以把RTSP控制信息和媒体数据流交织在一起传送，但一般情况**RTSP本身并不用于转送媒体流数据**。媒体数据的传送可通过RTP/RTCP等协议来完成。

该协议用于**C/S模型**, 是一个基于文本的协议, 用于在客户端和服务器端建立和协商实时流会话.

![img](./RTSP.assets/bc14f514cd04846e1ae80737b150745d.png)



### RTSP的工作原理

#### 1. 客户端-服务器模型

RTSP基于**客户端-服务器**模型，客户端向服务器发送请求控制服务器上的流媒体。RTSP会话**通常在TCP连接上进行**，但也**可以通过UDP或TCP/UDP混合方式**进行传输。

#### 2. 请求-响应模型

RTSP请求和响应采用**文本格式**，类似于HTTP的请求-响应模型，客户端发送请求，服务器返回响应。常见的请求包括：

- **OPTIONS**：查询服务器支持的方法和功能。
- **DESCRIBE**：获取媒体资源的描述信息，如编码格式、分辨率等。
- **SETUP**：设置媒体流传输的参数，如传输协议（UDP或TCP）、端口号等。
- **PLAY**：开始播放媒体流。
- **PAUSE**：暂停播放。
- **TEARDOWN**：结束会话，释放资源。

RTSP使用**状态代码**指示请求的执行状态，如200 OK表示成功，404 Not Found表示资源未找到。

#### 3. 媒体描述和SDP

RTSP使用SDP（Session Description Protocol）描述媒体流的属性和参数，包括流的URL、编码格式、媒体类型等。客户端通过DESCRIBE请求获取SDP信息，以便设置和播放媒体流。

#### 4. 会话控制

每个RTSP会话都有一个唯一的会话标识符（Session ID），用于管理和控制会话。

RTSP允许客户端控制媒体流的播放和会话的管理。通过SETUP、PLAY、PAUSE和TEARDOWN等请求，客户端可以启动、暂停、恢复和终止媒体流的传输会话。

### RTSP的优势

1. **灵活性**：RTSP支持多种媒体格式和编码方式，可以适应各种实时流媒体应用，如视频点播、实时视频直播等。
2. **控制能力**：RTSP允许客户端对流媒体进行精细的控制，如播放、暂停、快进、快退等操作，提供了与用户交互的能力。
3. **与其他协议的结合**：RTSP常与RTP（用于传输媒体数据）、RTCP（用于控制和监控RTP流）一起使用，共同构建完整的实时传输系统。
4. **广泛支持**：RTSP被广泛应用于视频监控系统、视频会议、IP电话、多媒体流媒体服务器等领域，成为实时媒体传输的标准之一。

### RTSP的不足

1. **传输效率**：RTSP本身不传输媒体数据，而是用于控制和管理媒体流。实际的媒体数据传输通常由RTP完成，因此在传输效率上需要额外考虑。
2. **安全性**：RTSP最初设计时未考虑安全性问题，如身份验证和加密，因此在安全性方面可能需要额外的保护措施来保护传输的媒体内容和会话信息。
3. **带宽需求**：RTSP的控制消息和SDP描述信息需要占用一定的带宽资源，特别是在大规模媒体流的传输中，可能对网络带宽造成一定的压力。

## 协议格式

### RTSP客户端的请求格式

```erlang
method url vesion\r\n
CSeq: x\r\n
xxx\r\n
...
\r\n
```

- method：方法，表明这次请求的方法，rtsp定义了很多方法，稍后介绍
- url：格式一般为rtsp://ip:port/session，ip表主机ip，port表端口好，如果不写那么就是默认端口，rtsp的默认端口为554，session表明请求哪一个会话
- version：表示rtsp的版本，现在为RTSP/1.0
- CSeq：序列号，每个RTSP请求和响应都对应一个序列号，序列号是递增的

### RTSP服务端的响应格式

```erlang
vesion 200 OK\r\n
CSeq: x\r\n
xxx\r\n
...
\r\n
```

- version：表示rtsp的版本，现在为RTSP/1.0
- CSeq：序列号，这个必须与对应请求的序列号相同

## RTSP的方法

|     方法      | 对象 | 方向      | 要求 |                             含义                             |
| :-----------: | :--: | --------- | :--: | :----------------------------------------------------------: |
|   DESCRIBE    | P,S  | C->S      | 推荐 | 检查演示或媒体对象的描述，也允许使用接收头指定用户理解的描述格式。DESCRIBE的答复-响应组成媒体RTSP初始阶段 |
|   ANNOUNCE    | P,S  | C->S S->C | 可选 | 当从用户发往服务器时，ANNOUNCE将请求URL识别的演示或媒体对象描述发送给服务器；反之，ANNOUNCE实时更新连接描述。如新媒体流加入演示，整个演示描述再次发送，而不仅仅是附加组件，使组件能被删除 |
| GET_PARAMETER | P,S  | C->S S->C | 可选 | GET_PARAMETER请求检查URL指定的演示与媒体的参数值。没有实体时，GET_PARAMETER也许能用来测试用户与服务器的连通情况 |
|    OPTIONS    | P,S  | C->S S->C | 要求 | 可在任意时刻发出OPTIONS请求，如用户打算尝试非标准请求，并不影响服务器状态 |
|     PAUSE     | P,S  | C->S      | 推荐 | PAUSE请求引起流发送临时中断。如请求URL命名一个流，仅回放和记录被停止；如请求URL命名一个演示或流组，演示或组中所有当前活动的流发送都停止。恢复回放或记录后，必须维持同步。在SETUP消息中连接头超时参数所指定时段期间被暂停后，尽管服务器可能关闭连接并释放资源，但服务器资源会被预订 |
|     PLAY      | P,S  | C->S      | 要求 | PLAY告诉服务器以SETUP指定的机制开始发送数据；直到一些SETUP请求被成功响应，客户端才可发布PLAY请求。PLAY请求将正常播放时间设置在所指定范围的起始处，发送流数据直到范围的结束处。PLAY请求可排成队列，服务器将PLAY请求排成队列，顺序执行 |
|    RECORD     | P,S  | C->S      | 可选 | 该方法根据演示描述初始化媒体数据记录范围，时标反映开始和结束时间；如没有给出时间范围，使用演示描述提供的开始和结束时间。如连接已经启动，立即开始记录，服务器数据请求URL或其他URL决定是否存储记录的数据；如服务器没有使用URL请求，响应应为201（创建），并包含描述请求状态和参考新资源的实体与位置头。支持现场演示记录的媒体服务器必须支持时钟范围格式，smpte格式没有意义 |
|   REDIRECT    | P,S  | C->S      | 可选 | 重定向请求通知客户端连接到另一服务器地址。它包含强制头地址，指示客户端发布URL请求；也可能包括参数范围，以指明重定向何时生效。若客户端要继续发送或接收URL媒体，客户端必须对当前连接发送TEARDOWN请求，而对指定主执新连接发送SETUP请求 |
|     SETUP     |  S   | C->S      | 要求 | 对URL的SETUP请求指定用于流媒体的传输机制。客户端对正播放的流发布一个SETUP请求，以改变服务器允许的传输参数。如不允许这样做，响应错误为"455 Method Not Valid In This State”。为了透过防火墙，客户端必须指明传输参数，即使对这些参数没有影响 |
| SET_PARAMETER | P,S  | C->S S->C | 可选 | 请求设置演示或URL指定流的参数值。请求仅应包含单个参数，允许客户端决定某个特殊请求为何失败。如请求包含多个参数，所有参数可成功设置，服务器必须只对该请求起作用。服务器必须允许参数可重复设置成同一值，但不让改变参数值。注意：媒体流传输参数必须用SETUP命令设置。将设置传输参数限制为SETUP有利于防火墙。将参数划分成规则排列形式，结果有更多有意义的错误指示 |
|   TEARDOWN    | P,S  | C->S      | 要求 | TEARDOWN请求停止给定URL流发送，释放相关资源。如URL是此演示URL，任何RTSP连接标识不再有效。除非全部传输参数是连接描述定义的，SETUP请求必须在连接可再次播放前发布 |

> 注：P---演示，C---客户端,S---服务器， S（对象栏）---流

### OPTION方法

请求及响应实例如下：

```http
OPTIONS rtsp://10.45.12.141:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 2
User-Agent: LibVLC/3.0.12 (LIVE555 Streaming Media v2016.11.28)

```

```http
RTSP/1.0 200 OK
CSeq: 2
Public: OPTIONS, DESCRIBE, PLAY, PAUSE, SETUP, TEARDOWN, SET_PARAMETER, GET_PARAMETER
Date:  Wed, Jul 27 2022 10:37:06 GMT
12345678
```

此方法主要用来询问流媒体服务器支持哪些RTSP方法，此例子说明服务器支持OPTIONS, DESCRIBE, PLAY, PAUSE, SETUP, TEARDOWN, SET_PARAMETER, GET_PARAMETER，此方法不是交互必须的，客户端可以跳过此方法直接发describe，服务器需要注意兼容。

### DESCRIBE方法

从服务器获取媒体流相关的信息，可以包含多个媒体流类型极信息，通过SDP来描述和区分不通媒体类型的媒体源，客户端根据服务器支持的媒体源通过setup建立媒体流通道，其实例如下：

```http
DESCRIBE rtsp://10.45.12.141:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 3
User-Agent: LibVLC/3.0.12 (LIVE555 Streaming Media v2016.11.28)
Accept: application/sdp

```

```http
RTSP/1.0 401 Unauthorized
CSeq: 3
WWW-Authenticate: Digest realm="IP Camera(C7627)", nonce="c4c4e29b1620211e44ec28b077e2eb52", stale="FALSE"
Date:  Wed, Jul 27 2022 10:37:06 GMT

```

```http
DESCRIBE rtsp://10.45.12.141:554/h264/ch1/main/av_stream RTSP/1.0
CSeq: 4
Authorization: Digest username="admin", realm="IP Camera(C7627)", nonce="c4c4e29b1620211e44ec28b077e2eb52", uri="rtsp://10.45.12.141:554/h264/ch1/main/av_stream", response="be7cde07af4a08db991dd58a89db7621"
User-Agent: LibVLC/3.0.12 (LIVE555 Streaming Media v2016.11.28)
Accept: application/sdp

```

```http
RTSP/1.0 200 OK
CSeq: 4
Content-Type: application/sdp
Content-Base: rtsp://10.45.12.141:554/h264/ch1/main/av_stream/
Content-Length: 574

v=0
o=- 1658918226996159 1658918226996159 IN IP4 10.45.12.141
s=Media Presentation
e=NONE
b=AS:5050
t=0 0
a=control:rtsp://10.45.12.141:554/h264/ch1/main/av_stream/
m=video 0 RTP/AVP 96
c=IN IP4 0.0.0.0
b=AS:5000
a=recvonly
a=x-dimensions:1920,1080
a=control:rtsp://10.45.12.141:554/h264/ch1/main/av_stream/trackID=1
a=rtpmap:96 H265/90000
a=fmtp:96 sprop-sps=QgEBAWAAAAMAsAAAAwAAAwB7oAPAgBDlja5JMvTcBAQEAg==; sprop-pps=RAHA8vA8kAA=
a=Media_header:MEDIAINFO=494D4B48010300000400050000000000000000000000000000000000000000000000000000000000;
a=appversion:1.0
123456789101112131415161718192021222324252627282930313233343536373839
```

这里客户端发送describe时，一般服务器会进行用户鉴权，如果未携带Authorization鉴权信息，或者认证失败，服务器会返回错误号为401的响应，客户端接收到401响应时，需要根据已知的用户鉴权信息，生成Authorization，再次发送describe，如果认证成功，服务器返回携带有SDP的响应信息。

有关服务器端SDP描述，这里提醒一下，fmtp要根据码流的真实信息填写，不要随意填写，在网页无插件播放时，越来越多的播放插件对这个字段要求很严格，因为网页客户端解码时，一般通过此字段来初始化解码库，所以此字段的填写，需要注意，要根据真实的编码参数来编写。

### SETUP方法

**RTP OVER UDP抓包实例：**

```http
SETUP rtsp://10.45.12.141:554/h264/ch1/main/av_stream/trackID=1 RTSP/1.0
CSeq: 5
Authorization: Digest username="admin", realm="IP Camera(C7627)", nonce="c4c4e29b1620211e44ec28b077e2eb52", uri="rtsp://10.45.12.141:554/h264/ch1/main/av_stream/", response="ac52cf287fe4aa6be5bb168bc9d01446"
User-Agent: LibVLC/3.0.12 (LIVE555 Streaming Media v2016.11.28)
Transport: RTP/AVP;unicast;client_port=63538-63539

```

```http
RTSP/1.0 200 OK
CSeq: 5
Session: 1279114011;timeout=60
Transport: RTP/AVP;unicast;client_port=63538-63539;server_port=8312-8313;ssrc=3cc5faf7;mode="play"
Date:  Wed, Jul 27 2022 10:37:07 GMT
1234567891011
```

首先客户端侧，SETUP的path路径，加上了trackID=1,表示建立的时trackID=1的媒体源的码流传输通道，通过上文SDP描述可知，此路时编码类型为H265，payloadtype=96的视频源，这里要注意，**一个setup只能给一种媒体源建立流传输通道**。如果服务器需要认证，setup也需要带上认证信息Authorization，Transport字段表示客户端通过何种方式申请建立媒体流传输通道，这里RTP/AVP表示通过UDP方式;unicast表示单播方式(也支持组播，比较少用，这里不做介绍)，如果为UDP方式，则有client_port字段，client_port=63538-63539表示客户端测此路码流的RTP端口为UDP63538，RTCP端口为UDP63539；这里注意建立RTP码流传输通道时，RTP和RTCP要成对出现，一般码流端口号为RTCP=RTP+1

服务器响应时，如果支持客户端的传输方式，则Transport字段中，RTP/AVP;unicast；client_port=63538-63539;要与客户端申请的消息保持一致，增加server_port字段，表示服务器发送RTP/RTCP的UDP端口，可选增加ssrc标识。这里要注意，服务端回复setup时，将会生成一个session ID.后续消息必须带有此Session字段。

**RTP OVER TCP抓包实例：**

```http
SETUP rtsp://10.45.12.141:554/h264/ch1/main/av_stream/trackID=1 RTSP/1.0
CSeq: 5
Authorization: Digest username="admin", realm="IP Camera(C7627)", nonce="59db6d7ba1acb14582356c8bb8e61ce8", uri="rtsp://10.45.12.141:554/h264/ch1/main/av_stream/", response="129818708e48cd0326f8b6f1b19613a3"
User-Agent: LibVLC/3.0.12 (LIVE555 Streaming Media v2016.11.28)
Transport: RTP/AVP/TCP;unicast;interleaved=0-1

```

```http
RTSP/1.0 200 OK
CSeq: 5
Session: 2095163832;timeout=60
Transport: RTP/AVP/TCP;unicast;interleaved=0-1;ssrc=24e4e500;mode="play"
Date:  Fri, Aug 26 2022 14:35:46 GMT
```

RTP通过TCP传输时，与UDP方式在SETUP方法上有一定的区别，主要是Transport头，RTP/AVP/TCP表示RTP流通过TCP传输，当此值出现时，其**没有client_port字段，出现interleaved字段**

interleaved=0-1，表示streamid，前文已经介绍了，当码流通过TCP传输时，与RTSP共用一个TCP链路，所以其不需要建立新的连接，为了区分RTP、RTCP及RTSP协议，需要增加包头标识，这里采用TCPHEAD头字段，tcphead为四个字节，格式如下：

| magic number | channel number | embedded data length | data |
| :----------: | :------------: | :------------------: | :--: |

- **magic number** ： 1 Byte value of hex 0x24（$），标识传输的是数据不是rtsp协议
- **channel number** ： 1 Byte value to denote the channel，信道ID，标识流的类型
- **embedded data length** ：2 Bytes to denote the embedded data length，流长度，单位是字节
- **data** :  表示RTP/RTCP包数据

当TCP接收到的包开头为24时，可以判定其为RTP或者RTCP，通过streamid来却分，setup方法中interleaved=0-1，标识RTP的streamid=0；RTCP的streamid=1

tcphead抓包实例如下：

![tcphead](./RTSP.assets/6b74d04403ec335fa30db2280df1cad7-1722243447124-1.png)

第一个24000014标识此包为RTP包长度为0x14，解析时只需要根据streamid及长度读取完整的RTP帧，去掉四字节头，通过RTP方式解析即可。由于TCP时流式传输，需要连续根据24标识判断。

### PLAY方法

PLAY消息是告诉服务器端可以使用在SETUP消息中所指定的传输机置开始传送数据。需要指出的是，客户端不应该发送任何PLAY请求直到所有的SETUP消息被成功解析。PLAY消息会在range中指定媒体的播放时间，服务器在接到PLAY消息后会由range中指定的开始点开始发送媒体数据直到range中指定的结束点，PlAY可带有scale和speed字段用于点播速度控制。对于实时流range一般为Range: npt=0.000

```http
PLAY rtsp://10.45.12.141:554/h264/ch1/main/av_stream/ RTSP/1.0
CSeq: 6
Authorization: Digest username="admin", realm="IP Camera(C7627)", nonce="59db6d7ba1acb14582356c8bb8e61ce8", uri="rtsp://10.45.12.141:554/h264/ch1/main/av_stream/", response="0eef224891c12e902ca9185c70f969cc"
User-Agent: LibVLC/3.0.12 (LIVE555 Streaming Media v2016.11.28)
Session: 2095163832
Range: npt=0.000-

```

```http
RTSP/1.0 200 OK
CSeq: 6
Session:       2095163832
RTP-Info: url=rtsp://10.45.12.141:554/h264/ch1/main/av_stream/trackID=1;seq=29458;rtptime=2518517708
Date:  Fri, Aug 26 2022 14:35:46 GMT
```

play方法需要带上Session字段表示统一会话，此Session由setup时服务端返回，客户端发送play方法后，即可准备接收码流，服务器接收到play后，即可打开码流发送通道，发送码流；这里要注意服务器在给出play响应时，最好带有RTP-Info字段描述将要发送码流的RTP信息，比如第一包RTP的seq和rtptime，客户端可以根据此字段进行解复用

### TEARDOWN方法

客户端发起表示停止媒体占用，并释放相关资源，其实例如下：

```http
TEARDOWN rtsp://10.45.12.141:554/h264/ch1/main/av_stream/ RTSP/1.0
CSeq: 7
Authorization: Digest username="admin", realm="IP Camera(C7627)", nonce="e3dfa4549e00a1d53c0e9f28c3348e2c", uri="rtsp://10.45.12.141:554/h264/ch1/main/av_stream/", response="0c530cba910c33ea3ef7a554dda8d0b2"
User-Agent: LibVLC/3.0.12 (LIVE555 Streaming Media v2016.11.28)
Session: 391346974
12345
```

这里teardown一般会停止RTSP会话及视频传输通道，服务器接收到此方法时，释放相关资源

### 其他方法

1. PAUSE方法：录像回放时会用到，用以暂停流媒体传输
2. SET_PARAMETER/GET_PARAMETER，此方法基本没啥用，一般用来作为心跳使用，也可用option来维持心跳

## RTSP URL 的格式

```c
rtsp_url = 
(”rtsp:” | ”rtspu:” | ”rtsps:”) “//” host [“:”port”] /[abs_path]/content_name
```

- rtsp:**使用可信的底层传输协议，例如TCP**
- rtspu:**使用不可信的底层传输协议，例如UDP**
- rtsps:**使用可信加密传输协议，例如TCP + TLS**
- host：可以是一个有效的域名或是IP地址。
- port：端口号，对于RTSP协议来说，**缺省的端口号为554**。当我们在确认流媒体服务器提供的端口号为554时，此项可以省略 说明：当HMS服务器使用的端口号为554时，我们在写点播链接时，可以不用写明端口号，但当使用非554端口时，在RTSP URL中一定要指定相应的端口。
- abs_path: 为RTSPServer中的媒体流资源标识
- RTSP URL用来标识RTSP Server的媒体流资源，可以标识单一的媒体流资源，也可以标 识多个媒体流资源的集合。

例如，一个完整的RTSP URL可写为：

```c
rtsp://192.168.1.67:554/test
```

又如目前市面上常用的海康网络摄像头的RTSP地址格式为：

```c
rtsp://[username]:[password]@[ip]:[port]/[codec]/[channel]/[subtype]/av_stream
```

示例

```c
rtsp://admin:12345@192.168.1.67:554/h264/ch1/main/av_stream
```

另一个简单的示例如下：

```c
rtsp://media.example.com:554/twister/audiotrack
```

让我们来看一下上面URL的abs path = twister/audiotrack。twister表示一个标识(Presentation) ，标识(Presentation)由一个或多个实时流组成。audiotrack表示标识(Presentation)中其中一个实时流的名称。从这个名称可以看出，我们要取的是一个音频流。如果abs path = twister/videotrack，则表示我们要取的是twister的视频流。

有的服务器也支持下面的URL形式：

```c
rtsp://media.example.com:554/twister
```

该URL表示取标识(Presentation)的视频流和音频流。

## RTSP重要头字段参数

**Accept**

- 指定客户端可以接受的媒体描述信息类型。示例: `Accept: application/rtsl, application/sdp;level=2`

**Bandwidth**

- 描述客户端可用的带宽值，以千比特每秒（kbps）为单位。示例: `Bandwidth: 5000`

**Authorization**:

- 用于提供认证信息，例如Basic或Digest认证。示例：`Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==`

**Cache-Control**:

- 用于指示缓存行为。示例：`Cache-Control: no-cache`

**Connection**:

- 控制网络连接管理，例如保持连接或关闭连接。示例：`Connection: close`

**Content-Base**:

- 指定内容基础URL，用于解析相对URL。示例：`Content-Base: rtsp://example.com/media/`

**Content-Encoding**:

- 指定内容编码方式，例如gzip。示例：`Content-Encoding: gzip`

**Content-Language**:

- 指定内容的语言。示例：`Content-Language: en-US`

**Content-Length**:

- 指定消息体的长度（以字节为单位）。示例：`Content-Length: 1234`

**Content-Type**:

- 指定消息体的媒体类型。示例：`Content-Type: application/sdp`

**CSeq**

- 指定RTSP请求和回应对的序列号。每个请求和对应的回应必须包含这个头字段。示例: `CSeq: 2`

**Date**:

- 指定消息的创建时间和日期。示例：`Date: Mon, 20 Jul 2024 15:23:31 GMT`

**Expires**:

- 指定响应消息过期时间。示例：`Expires: Mon, 20 Jul 2024 16:23:31 GMT`

**From**:

- 指定客户端用户的电子邮件地址。示例：`From: user@example.com`

**If-Modified-Since**:

- 请求只有在指定日期之后修改的资源。示例：`If-Modified-Since: Sat, 29 Jul 2024 12:00:00 GMT`

**Last-Modified**:

- 指定资源最后修改的日期和时间。示例：`Last-Modified: Sat, 29 Jul 2024 12:00:00 GMT`

**Proxy-Authenticate**:

- 用于代理服务器请求客户端认证。示例：`Proxy-Authenticate: Basic realm="Example"`

**Proxy-Authorization**:

- 用于客户端提供给代理服务器的认证信息。示例：`Proxy-Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==`

**Public**:

- 指定服务器支持的RTSP方法。示例：`Public: OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN`

**Range**:

- 指定一个时间范围，可以使用SMPTE、NTP或clock时间单元。示例：`Range: npt=0-10`

**Referer**:

- 指定引导请求的URL。示例：`Referer: rtsp://example.com/previous`

**Retry-After**:

- 指定客户端应在多长时间后重试请求。示例：`Retry-After: 120`

**Scale**:

- 指定流媒体的播放速度。示例：`Scale: 1.5`

**Server**:

- 指定服务器软件信息。示例：`Server: RTSP Server/1.0`

**Session**

- 标识一个RTSP会话。Session ID由服务器在SETUP回应中选择，客户端在以后的请求消息中必须包含这个ID。示例: `Session: 12345678`

**Speed**:

- 指定流媒体的播放速度。示例：`Speed: 2.0`

**Transport**:

- 包含客户端可以接受的转输选项列表，包括传输协议，地址端口，TTL等。服务器端也通过这个头字段返回实际选择的具体选项。示例：`Transport: RTP/AVP;unicast;client_port=3456-3457`

**Unsupported**:

- 指定服务器不支持的RTSP扩展。示例：`Unsupported: org.w3c.xmr`

**User-Agent**:

- 指定客户端软件信息。示例：`User-Agent: RTSP Client/1.0`

**Vary**:

- 指定影响响应的请求头字段。示例：`Vary: Accept-Encoding`

**Via**:

- 指定中间代理或网关信息。示例：`Via: 1.0 proxy1, 1.1 proxy2`

**WWW-Authenticate**:

- 用于服务器请求客户端认证。示例：`WWW-Authenticate: Basic realm="Example"`



## RTSP与HTTP

RTSP（实时流协议）和HTTP（超文本传输协议）是用于不同类型数据传输的两种协议，各有其优势和缺点。下面是对RTSP和HTTP的比较：

**RTSP的优势**

1. **实时性**：
   - RTSP专为实时流媒体传输设计，支持实时音视频的点播和直播。
   - 它允许客户端控制流媒体的播放，如播放、暂停、停止、快进和快退等操作。
2. **低延迟**：
   - RTSP通常与RTP（实时传输协议）结合使用，能够实现低延迟的音视频传输，非常适合需要实时互动的应用场景，如视频会议和IPTV。
3. **带宽利用率**：
   - RTSP可以动态调整码率以适应网络带宽的变化，优化带宽利用率，提供更流畅的观看体验。
4. **多媒体控制**：
   - RTSP提供丰富的控制命令，允许客户端在流媒体播放过程中精确控制播放状态，适合需要频繁操作和交互的应用。
5. **状态控制：**
   - RTSP为每个会话保持状态。

**RTSP的缺点**

1. **复杂性**：
   - RTSP协议较为复杂，实施和维护需要较高的技术水平。
   - 比起HTTP，RTSP需要更多的资源来处理控制信令和媒体流的同步。
2. **兼容性**：
   - RTSP在浏览器中的支持有限，通常需要专用的客户端软件或插件才能播放。
   - 相对于HTTP，RTSP的生态系统较小，不如HTTP广泛普及。
3. **防火墙穿透问题**：
   - RTSP使用多端口通信（通常使用554端口进行信令控制，其他端口进行媒体传输），可能会被防火墙阻止，导致穿透性较差。

**HTTP的优势**

1. **普及性**：
   - HTTP是最常用的网络协议之一，几乎所有的浏览器和设备都支持HTTP。
   - 其生态系统庞大，开发和调试工具丰富。
2. **简单性**：
   - HTTP协议相对简单，实施和维护成本较低。
   - 大量的现成库和框架可以简化开发工作。
3. **防火墙穿透**：
   - HTTP通常使用单一的80或443端口进行通信，更容易通过防火墙和网络安全设备。

**HTTP的缺点**

1. **实时性不足**：
   - HTTP主要为静态内容传输设计，采用请求-响应模型，每次客户端发送请求后，服务器响应并传输完整的内容，不适合实时数据传输。
   - 在直播和实时交互应用中，延迟较高。HTTP的通信是单向的，客户端发送请求，服务器响应。这种单向通信不适合需要频繁双向交互的流媒体控制。
2. **带宽利用率低**：
   - HTTP传输的开销较大，对于大规模、高频率的数据传输（如视频流）效率不高。
3. **缺乏控制功能**：
   - HTTP缺乏内置的流媒体控制信令，无法直接支持播放、暂停、快进和快退等操作。

**总结**

RTSP适用于需要低延迟、实时控制的流媒体应用，如视频会议、IPTV和实时监控。HTTP则适用于常规的网页浏览和文件传输，尤其是在兼容性和防火墙穿透性方面有显著优势。在实际应用中，可以根据具体需求选择合适的协议或将两者结合使用，如通过HTTP进行初始化，然后使用RTSP进行实时传输。

## 示例：实时视频监控系统（RTSP+RTCP+RTP）

**背景**

设想一个实时视频监控系统，其中多个摄像头将视频流实时传输到监控中心。在这个系统中，我们希望能够控制每个摄像头的播放状态（如开始、暂停、停止），并实时监控传输质量以确保视频流的流畅和稳定。



**系统组成**

1. **摄像头设备**：负责捕捉视频并通过RTP传输视频数据。
2. **RTSP服务器**：运行在监控中心，处理RTSP请求，控制摄像头的播放状态。
3. **监控客户端**：连接到RTSP服务器，发送控制命令，接收并显示视频流。
4. **网络**：传输视频数据和控制信息。



**工作流程**

1. **初始化连接**
   - 监控客户端通过RTSP向RTSP服务器发送`OPTIONS`请求，查询服务器支持的功能。
   - RTSP服务器响应支持的命令，如`DESCRIBE`、`SETUP`、`PLAY`、`PAUSE`、`TEARDOWN`等。
2. **获取媒体描述**
   - 客户端发送`DESCRIBE`请求，获取摄像头视频流的描述信息（如视频格式、编码类型、比特率等）。
   - RTSP服务器返回包含SDP（Session Description Protocol）的响应，描述视频流的详细信息。
3. **设置传输参数**
   - 客户端发送`SETUP`请求，设置视频流的传输参数（如RTP/UDP传输）。
   - RTSP服务器响应并建立会话，分配会话ID。
4. **开始播放**
   - 客户端发送`PLAY`请求，开始接收视频流。
   - RTSP服务器通知摄像头通过RTP传输视频数据。
5. **传输视频数据**
   - 摄像头通过RTP将视频数据包发送到客户端。RTP数据包包含序列号和时间戳，以确保数据包的有序传输和同步。
   - 同时，摄像头和客户端通过RTCP交换控制报文，监控传输质量。摄像头发送`Sender Report (SR)`，报告发送数据的统计信息；客户端发送`Receiver Report (RR)`，报告接收数据的质量，如丢包率、延迟等。
6. **暂停/停止播放**
   - 客户端发送`PAUSE`请求，暂停视频流。
   - 客户端发送`TEARDOWN`请求，终止会话并释放资源。



### **具体示例流程**

1. **初始化连接**

```
OPTIONS rtsp://camera.example.com/stream1 RTSP/1.0
CSeq: 1
```

```
RTSP/1.0 200 OK
CSeq: 1
Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE
```

2. **获取媒体描述**

```
DESCRIBE rtsp://camera.example.com/stream1 RTSP/1.0
CSeq: 2
```

```
RTSP/1.0 200 OK
CSeq: 2
Content-Base: rtsp://camera.example.com/stream1/
Content-Type: application/sdp
Content-Length: 460

v=0
o=- 2890844526 2890842807 IN IP4 127.0.0.1
s=Example Stream
i=Example video stream
t=0 0
a=control:rtsp://camera.example.com/stream1/
m=video 0 RTP/AVP 96
c=IN IP4 0.0.0.0
a=rtpmap:96 H264/90000
```

3. **设置传输参数**

```
SETUP rtsp://camera.example.com/stream1/trackID=1 RTSP/1.0
CSeq: 3
Transport: RTP/AVP;unicast;client_port=8000-8001
```

```
RTSP/1.0 200 OK
CSeq: 3
Transport: RTP/AVP;unicast;client_port=8000-8001;server_port=9000-9001
Session: 12345678
```

4. **开始播放**

```
PLAY rtsp://camera.example.com/stream1/ RTSP/1.0
CSeq: 4
Session: 12345678
```

```
RTSP/1.0 200 OK
CSeq: 4
Session: 12345678
RTP-Info: url=rtsp://camera.example.com/stream1/trackID=1;seq=9810092;rtptime=3450012
```

5. **传输视频数据**

摄像头通过RTP发送视频数据包，客户端接收并解码显示。

RTCP报文交换：

```
Sender Report (SR):
v=2 p=0 count=1 packet_type=200 length=6
sender_ssrc=1234
ntp_timestamp=3633244821
rtp_timestamp=3450012
packet_count=345
octet_count=123456
```

```
Receiver Report (RR):
v=2 p=0 count=1 packet_type=201 length=7
ssrc=1234
fraction_lost=0 cumulative_lost=0
highest_seq_no=9810092
jitter=5
lsr=0
dlsr=0
```

6. **暂停/停止播放**

```
PAUSE rtsp://camera.example.com/stream1/ RTSP/1.0
CSeq: 5
Session: 12345678
```

```
RTSP/1.0 200 OK
CSeq: 5
Session: 12345678
```



```
TEARDOWN rtsp://camera.example.com/stream1/ RTSP/1.0
CSeq: 6
Session: 12345678
```

```
RTSP/1.0 200 OK
CSeq: 6
Session: 12345678
```

**说明**

- **RTSP**：负责控制摄像头的视频流，包括开始、暂停、停止等操作。
- **RTP**：负责传输实际的视频数据包。
- **RTCP**：负责监控视频流的传输质量，提供反馈以确保流的质量和同步。

## 连接过程解析

[链接](https://www.xianwaizhiyin.net/?p=1574)

[包文件](./RTSP.assets/rtsp-推流.pcapng)