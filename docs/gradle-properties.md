# Android Gradle 代理配置与重试步骤（PowerShell）

## 1. 在当前 PowerShell 窗口设置代理

```powershell
$env:HTTP_PROXY  = "http://127.0.0.1:9910"
$env:HTTPS_PROXY = "http://127.0.0.1:9910"
```

说明：这两行只对“当前终端窗口”生效。

## 2. 把 Gradle 代理写入 `gradle.properties`

先打开用户级 Gradle 配置文件：

```powershell
notepad "$env:USERPROFILE\.gradle\gradle.properties"
```

在文件末尾追加下面内容并保存：

```properties
systemProp.http.proxyHost=127.0.0.1
systemProp.http.proxyPort=9910
systemProp.https.proxyHost=127.0.0.1
systemProp.https.proxyPort=9910
systemProp.http.nonProxyHosts=localhost|127.0.0.1
systemProp.https.nonProxyHosts=localhost|127.0.0.1
```

注意：`systemProp...` 这些是配置文件内容，不是 PowerShell 命令，不能直接在终端输入执行。

## 3. 验证代理是否可用

```powershell
curl -x http://127.0.0.1:9910 -I https://dl.google.com/dl/android/maven2/
```

如果返回 `HTTP 200/301/302` 等状态码，说明链路基本可用。

## 4. 重启 Gradle Daemon 并重试构建

```powershell
cd C:\Computer\my_github\github_cpp\workout_calculator\workout_calculator\apps\workout_android
.\gradlew --stop
.\gradlew :app:assembleDebug --refresh-dependencies
```

## 5. 若仍失败，继续排查

执行并保存输出：

```powershell
netstat -ano | findstr :9910
curl -x http://127.0.0.1:9910 -I https://www.google.com
```

把输出贴给我，继续定位是本地代理、证书还是网络策略问题。
