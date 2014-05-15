HIME Input Method Editor
========================

Hime 新手上路
=============

簡介
----

Hime 是一個極好用的輸入法框架, 輕巧、穩定、功能強大且支援許多常用的輸入法,包括倉頡, 注音，大易，行列，嘸蝦米 ,希臘字母,
日本 Anthy, 韓拼, 拉丁字母, 亂倉打鳥, 酷音等....

### 特色

- 支援多種輸入法, 使用者選擇多
- 支援多種智慧型注音輸入法
- 支援多種拆字型輸入法
- 支援內碼輸入

### 授權

LGPLv2.1 (Qt immodules are GPLv2)


取得/安裝
---------

如果您尚未取得 Hime, 可依下列方法取得 Hime:


#### 最新開發版

    git clone https://github.com/hime-ime/hime.git
    cd hime


#### deb系

    distro/debian/gen-deb


#### rpm系

    distro/fedora/gen-rpm


#### Debian/Ubuntu

    apt-get update
    apt-get install hime


#### Archlinux


    # install `hime-git` from AUR, thanks to @xatier
    yaourt hime-git

    # build from this repo
    cd distro/archlinux
    makepkg -s
    sudo pacman -U hime-git-{version}-{arch}.pkg.tar.xz


#### 自行編譯

    ./configure && make && make install



開始使用 Hime
=============

開啟/切換輸入法
---------------

- 按 Ctrl+Space 來開啟輸入視窗
- 按 Ctrl+Shift 來循環切換輸入法


Hime 問題回報
=============

使用者可以依下列途徑回報問題:

-   Github issue tracker
-   #hime at irc.freenode.net


附錄
====


常見問題
------

#### "HIME" 怎麼念？

-   HIME 在日語中是「姫」，公主的意思，發音為「ひめ」。中文沒有對應的音，英文的發音為: Pronounce "Hi" like the English word "he", then pronounce "me" like the English word "may" or the "Me" in "Merry Christmas"。


#### 為什麼（詞音的）詞庫不會隨著 HIME 更新？

-   目前輸入法的設計並不會更新存放於使用者家目錄的詞庫資料，未來的版本可能會改善，細節請見 issue #136


按鍵功能一覽表
--------------


    按鍵
    -------------------------- --------------------------
    *                          gtab 輸入法拆碼中代表任意數量字元 非 gtab 輸入法中輸出 * 或 ＊
    ?                          gtab 輸入法拆碼中代表任意單一字元 輸出?或？
    `                          gtab 輸入法中開啟同音字選擇視窗
    <                          注音輸入法顯示上一頁重覆字
    ’                          詞音中輸出全形、符號
    h                          詞音 vi 編輯模式遊標左移一個字元
    l                          詞音 vi 編輯模式遊標右移一個字元
    Q                          詞音許氏鍵盤排列時可選擇同音字(跟向下鍵一樣)
    q                          詞音許氏鍵盤排列時可選擇同音字(跟向下鍵一樣)
    x                          詞音 vi 編輯模式刪除一個字元
    Alt+Shift+按鍵             輸出 phrase.table 定義的字串
    Alt+Space                  可設定為輸入法狀態切換開關
    BackSpace                  清除一個拆碼 清除緩衝區的一個字元
    CapsLock                   詞音/日本 anthy 切換中英文狀態
    Ctrl+Alt+Space             在輸入視窗畫紅色的 X 用以協助除錯
    Ctrl+Alt+,                 符號視窗開關切換
    Ctrl+Alt+0                 切換為內碼輸入法
    Ctrl+Alt+3                 切換為注音輸入法
    Ctrl+Alt+6                 切換為詞音輸入法
    Ctrl+Alt+=                 切換為日本 anthy 輸入法
    Ctrl+Alt+g                 輸出前一次的字串
    Ctrl+Alt+r                 輸出前一次的字串
    Ctrl+Alt+【-1245789=[\]`】 切換為 gtab 輸入法
    Ctrl+Shift                 循環切換輸入法
    Ctrl+Shift+;               在非 XIM 模式輸出全形：符號
    Ctrl+Space                 輸入法狀態切換開關
    Ctrl+e                     詞音切換 vi 編輯模式
    Ctrl+u                     清除詞音緩衝區
    Ctrl+按鍵                  輸出 phrase-ctrl.table 定義的字串
    Delete                     刪除緩衝區遊標所在的字 刪除內碼輸入法的一個拆碼
    Down                       詞音中選擇同音字
    End                        關閉選擇視窗並移動到緩衝區末端
    Enter                      輸出緩衝區內容
    Escape                     清除所有拆碼 關閉gtab同音字選擇視窗
    F11                        日本 anthy 輸入法中呼叫 kasumi 管理模式
    F12                        日本 anthy 輸入法中呼叫 kasumi 加詞模式(先圈選想加的詞再按 F12)
    Home                       關閉選擇視窗並移動到緩衝區最前端
    Left                       關閉選擇視窗並在緩衝區左移一個字元
    PageDown                   gtab/日本 anthy
    PageUp                     gtab/日本 anthy
    數字盤的+鍵                gtab 輸入法顯示下一頁重覆字(若在末頁則回到第一頁)
    數字盤的-鍵                gtab 輸入法顯示上一頁重覆字(到第一頁為止)
    Right                      關閉選擇視窗並在緩衝區右移一個字元
    Shift                      可設定為詞音/日本 anthy 切換中英文狀態
    Shift+Enter                詞音新增詞(從遊標所在位置到緩衝區末端)
    Shift+Space                全形狀態切換開關 可設定為輸入法狀態切換開關
    Shift+按鍵                 可設定在 gtab 狀態中取代「Alt+Shift+按鍵」
    Shift+數字                 詞音中選擇候選詞
    Shift+標點                 詞音中輸出全形標點符號
    Space                      拆碼輸入完成顯示候選字 翻頁顯示重覆字 輸出第一個候選字   輸出半形或全形空白 注音及詞音狀態表示音調的一聲
    Tab                        可在詞音中代替 Enter 鍵輸出緩衝區內容 可設定為詞音切換中英文狀態
    Up                         詞音中選擇近音字
    Windows+Space              可設定為輸入法狀態切換開關
    滑鼠左鍵                   符號視窗開關切換 系統列半形全形開關切換
    滑鼠中鍵                   選擇輸入法
    滑鼠右鍵                   顯示系統列選單
    滑鼠滾輪往上               符號視窗循環切換成上一組符號表(滑鼠遊標須在符號視窗範圍內)
    滑鼠滾輪往下               符號視窗循環切換成下一組符號表(滑鼠遊標須在符號視窗範圍內)
    --------------- ------------------------------------

參考文獻
========

[取得/安裝](https://github.com/hime-ime/hime/wiki/Prebuilt-packages-for-Linux-distributions)
