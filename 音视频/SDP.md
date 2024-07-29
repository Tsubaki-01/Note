## SDP

会话描述协议（Session Description Protocol，SDP）是一种用于描述多媒体通信会话的格式，通常用于初始化和控制实时传输协议（RTP）流。SDP 描述是纯文本格式，采用 UTF-8 编码。以下是一个示例 SDP 描述，包含各个字段的详细解释：

### 示例 SDP 描述

```plaintext
v=0
o=johndoe 2890844526 2890842807 IN IP4 192.168.0.1
s=SDP Seminar
i=A Seminar on the session description protocol
u=http://www.example.com/seminars/sdp.pdf
e=johndoe@example.com
p=+1 617 555-6011
c=IN IP4 192.168.0.1
b=CT:384
t=2873397496 2873404696
r=604800 3600 0 90000
z=2882844526 -1h 2898848070 0
k=clear:password
a=recvonly
m=audio 49170 RTP/AVP 0
i=Audio Stream
c=IN IP4 192.168.0.1
b=AS:128
k=prompt
a=sendrecv
m=video 51372 RTP/AVP 99
a=rtpmap:99 h263-1998/90000
```

### 字段解释

#### 会话描述

- `v=` （协议版本）

  ```plaintext
  v=0
  ```

  表示 SDP 版本，目前固定为 0。

- `o=` （所有者/创建者和会话标识符）

  ```plaintext
  o=johndoe 2890844526 2890842807 IN IP4 192.168.0.1
  ```

  描述会话的创建者和会话的唯一标识符。格式为：用户名、会话 ID、版本号、网络类型、地址类型和地址。

- `s=` （会话名称）

  ```plaintext
  s=SDP Seminar
  ```

  会话的名称。

- `i=*` （会话信息，选填）

  ```plaintext
  i=A Seminar on the session description protocol
  ```

  会话的描述信息。

- `u=*` （URI 描述，选填）

  ```plaintext
  u=http://www.example.com/seminars/sdp.pdf
  ```

  会话的相关 URI。

- `e=*` （Email 地址，选填）

  ```plaintext
  e=johndoe@example.com
  ```

  会话联系人的电子邮件地址。

- `p=*` （电话号码，选填）

  ```plaintext
  p=+1 617 555-6011
  ```

  会话联系人的电话号码。

- `c=*` （连接信息，选填）

  ```plaintext
  c=IN IP4 192.168.0.1
  ```

  会话的连接信息。如果每个媒体描述中都有连接信息，则此字段可选。

- `b=*` （带宽信息，选填）

  ```plaintext
  b=CT:384
  ```

  会话的带宽信息。

#### 时间描述

- `t=` （会话活动时间）

  ```plaintext
  t=2873397496 2873404696
  ```

  会话的开始和结束时间，格式为 NTP 时间戳。

- `r=*` （重复次数，选填）

  ```plaintext
  r=604800 3600 0 90000
  ```

  重复时间和间隔。

- `z=*` （时间区域调整，选填）

  ```plaintext
  z=2882844526 -1h 2898848070 0
  ```

  时间区域调整信息。

- `k=*` （加密密钥，选填）

  ```plaintext
  k=clear:password
  ```

  会话的加密密钥。

#### 媒体描述

- `m=` （媒体名称和传输地址）

  ```plaintext
  m=audio 49170 RTP/AVP 0
  ```

  描述媒体类型、传输端口、传输协议和格式列表。

- `i=*` （媒体标题，选填）

  ```plaintext
  i=Audio Stream
  ```

  媒体流的标题。

- `c=*` （连接信息，选填）

  ```plaintext
  c=IN IP4 192.168.0.1
  ```

  媒体的连接信息。如果会话层已经提供连接信息，此字段可选。

- `b=*` （带宽信息，选填）

  ```plaintext
  b=AS:128
  ```

  媒体的带宽信息。

- `k=*` （加密密钥，选填）

  ```plaintext
  k=prompt
  ```

  媒体的加密密钥。

上述示例展示了一个完整的 SDP 描述，包括会话描述、时间描述和媒体描述。根据具体需求，可以添加或省略可选字段。

#### 常见的 `a=` 字段属性

1. **`a=rtpmap:<payload type> <encoding name>/<clock rate>`**
   - 说明：指定RTP负载类型（`<payload type>`）和对应的编码名称及其时钟频率（`<encoding name>/<clock rate>`）。
   - 示例：`a=rtpmap:96 VP8/90000`
2. **`a=fmtp:<payload type> <parameter>`**
   - 说明：为指定的负载类型提供附加格式参数。
   - 示例：`a=fmtp:96 max-fr=30; max-fs=1220`
3. **`a=control:<url>`**
   - 说明：指定用于控制该媒体流的URL。
   - 示例：`a=control:rtsp://example.com/stream`
4. **`a=ptime:<ptime>`**
   - 说明：指定音频媒体的每个数据包的时间长度（以毫秒为单位）。
   - 示例：`a=ptime:20`
5. **`a=maxptime:<maxptime>`**
   - 说明：指定音频媒体数据包的最大时间长度（以毫秒为单位）。
   - 示例：`a=maxptime:60`
6. **`a=recvonly`**
   - 说明：指定媒体流仅用于接收。
   - 示例：`a=recvonly`
7. **`a=sendonly`**
   - 说明：指定媒体流仅用于发送。
   - 示例：`a=sendonly`
8. **`a=sendrecv`**
   - 说明：指定媒体流用于发送和接收。
   - 示例：`a=sendrecv`
9. **`a=inactive`**
   - 说明：指定媒体流处于不活动状态，不接收也不发送数据。
   - 示例：`a=inactive`
10. **`a=rtcp:<port>`**
    - 说明：指定RTCP的端口。
    - 示例：`a=rtcp:5000`
11. **`a=ssrc:<ssrc-id>`**
    - 说明：指定RTP流的同步源标识符（SSRC）。
    - 示例：`a=ssrc:12345678 cname:stream1`
12. **`a=crypto:<crypto-suite> <key>`**
    - 说明：指定加密套件及其密钥，用于媒体流的加密。
    - 示例：`a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:4E1C9D3A3B9B5D7C5C4D2E2A4F6E1E2D`

#### 其他常见的 `a=` 字段

- **`a=label:<label>`**：为媒体流指定一个标签。
- **`a=quality:<value>`**：指定媒体流的质量。
- **`a=framerate:<framerate>`**：指定视频媒体流的帧率。



