# ソケットプログラミング

コードについての説明は[ここ](MEMO.md)を参照。

## 概要

C言語でソケットプログラミングを行った。クライアント・サーバー間はTCPで通信している。シェル、`wget`コマンド、HTTPサーバーを実装しており、簡単にだが、HTTPを喋ることができる。

## 環境構築

コンパイル：

```bash
make
```

サーバーの起動：

```bash
$ make runs
server starting...
listening on port 80
```

シェル（クライアント）の起動：

```bash
$ make runc
shell> 
```

## 使い方

### クライアント

サーバーとシェルを起動した後、シェルで下記のように実行するとサーバーにGETリクエストを送ることができる。

```bash
shell> wget localhost

HTTP/1.1 200 OK
Content-Type: text/html
Connection: close

<html><body><h1>Hello from server</h1></body></html>


shell>
```

また外部のサイトに対してもリクエストを送ることができる（HTTPのみ、パス指定は未対応）。

```bash
shell> wget www.google.com

HTTP/1.1 200 OK
Date: Sun, 06 Oct 2024 02:21:04 GMT
Expires: -1
Cache-Control: private, max-age=0
Content-Type: text/html; charset=ISO-8859-1
Content-Security-Policy-Report-Only: object-src 'none';base-uri 'self';script-src 'nonce-mr7B4KIQsSwcXXXV1Gah2g' 'strict-dynamic' 'report-sample' 'unsafe-eval' 'unsafe-inline' https: http:;report-uri https://csp.withgoogle.com/csp/gws/other-hp
P3P: CP="This is not a P3P policy! See g.co/p3phelp for more info."
Server: gws
X-XSS-Protection: 0
X-Frame-Options: SAMEORIGIN
Set-Cookie: AEC=AVYB7cq_JHauoM0q54kqbna0Wrh-aUGFlF836-FR4vNjr3aM_w3KuGuh0Q; expires=Fri, 04-Apr-2025 02:21:04 GMT; path=/; domain=.google.com; Secure; HttpOnly; SameSite=lax
Set-Cookie: NID=518=N_c6eKUvVmzd7pPQJfOIFZ8qVSZSFPFJArlfaRaD9x-VZ_YLZcBpAGfgiAHZS43q4A_E2fsUwuO3RsX5GvJ3tcTmxK0nJ87DOev7vOQecCHZBQMeDUt9VgqbdsQFq2Bzii0369USTf8YVRSxtMqUWSQ0_E48zJW9l1W3vOxNvNgs2CygFkm_tqAkV_I9kERx3DQ; expires=Mon, 07-Apr-2025 02:21:04 GMT; path=/; domain=.google.com; HttpOnly
Accept-Ranges: none
Vary: Acc
shell> 
```

### サーバー

サーバーはブラウザからもアクセスできる。

![ブラウザ](images/browser.png)

`Ctrl + C`でサーバーを停止できる。

```bash
% make runs
server starting...
listening on port 80
^Caccept failed: Interrupted system call
shutting down server...
server stopped.
```
