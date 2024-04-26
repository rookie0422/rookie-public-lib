# Ubuntu



[构建一个 Debian （'.deb'） 源码包，并将其发布在 Ubuntu PPA 上 ](https://saveriomiroddi.github.io/Building-a-debian-deb-source-package-and-publishing-it-on-an-ubuntu-ppa/)

[从零开始制作 deb 文件 ](https://hedzr.com/packaging/deb/creating-deb-file-from-scratch/)

## 设置

首先，我们安装所需的工具：

```
$ sudo apt install gnupg dput dh-make devscripts lintian
```

### 设置和发布 PGP 密钥

Ubuntu 发布平台是 [Launchpad](https://launchpad.net/);对它的操作需要电子签名，因此第一件事是：

- [设置 PGP 密钥](https://help.ubuntu.com/community/GnuPrivacyGuardHowto)

  打开[终端](https://help.ubuntu.com/community/UsingTheTerminal)并输入：

  ```sh
  gpg --gen-key
  ```

Ubuntu 通过自己的密钥服务器验证密钥，因此接下来：

- 公钥必须上传到 Ubuntu 密钥服务器上：https://keyserver.ubuntu.com

### 设置 Launchpad 帐户

需要 Launchpad 帐户;它可以是：

- 在 [Launchpad 登录页面上](https://login.launchpad.net/)创建

创建时，将创建一个用户页面，该页面也是用户面板。

在这种情况下，关键的相关步骤是：

- 注册PGP密钥（通过编辑页面：https://launchpad.net/~barryfoo/+editpgpkeys）`OpenPGP keys`



要导入密钥，您需要密钥指纹。要列出所有密钥及其指纹，请输入：

```
gpg --fingerprint
```

指纹将如下所示：

```
95BD 8377 2644 DD4F 28B5  2C37 0F6E 4CA6 D8FC 66D2
```

要导入密钥，请执行以下操作：

- 如有必要，请登录 Launchpad。

- 访问“更改您的 OpenPGP 密钥”页面。

- 将指纹粘贴到文本框中，然后单击“导入密钥”。Launchpad 将向您发送一封加密电子邮件。电子邮件的文本将类似于以下内容：

  ```
  -----BEGIN PGP MESSAGE-----
  Version: GnuPG v1.4.3 (GNU/Linux)
  
  hQIOA0THhKozD+K5EAf9F3PcOL2iU6onH2YsvB6IKDXNxbK0NBVy6ppxcNq8hoTe
  cuHvzWLFfh1ehhSNe1V6xpuFnt5sJoeA4qEEOxez3HmY80tKIKMPLyhC/8JiSIW9
  
  [Lines deleted]
  
  V9E+hW6Qehx5DbsIXEvfeaBBHOfAHHOhUH14WK4bsJWm8wZ50XiYBZrNFOqzsm13
  2STcY4VIoJp3Uw2qNyvZXQUhpndlfgQGO14CMSadzDn6Vts=
  =hTe6
  -----END PGP MESSAGE-----
  ```

- 将电子邮件的文本保存到文件中。

- 解密文本：

  ```
  gpg --decrypt file.txt
  ```

- 输入您的密码。该消息将与您必须遵循的链接一起显示，以便使用 Launchpad 验证您的密钥。

- 点击链接，在出现提示时输入您的 Launchpad 密码，您就完成了！

### 创建 PPA

[ppa-dev-tools使用手册](https://git.launchpad.net/ppa-dev-tools/tree/README.md)

`ppa-dev-tools` 是一个用于在 Launchpad PPA 中进行开发和维护的工具集，但是它并不是通过 Snap 安装的。通常情况下，您可以通过以下步骤来安装 `ppa-dev-tools`：

1. **添加 PPA：** 首先，您需要将 `ppa:ppa-dev-tools/ppa` 添加到您的系统中。您可以使用以下命令添加该 PPA：

```bash
sudo add-apt-repository ppa:ppa-dev-tools/ppa
```

1. **更新软件包列表：** 添加 PPA 后，需要更新您的系统的软件包列表，以便识别新添加的软件包。您可以使用以下命令更新软件包列表：

```bash
sudo apt update
```

1. **安装 ppa-dev-tools：** 最后，您可以使用以下命令安装 `ppa-dev-tools`：

```bash
sudo apt install ppa-dev-tools
```

这样就会在您的系统中安装 `ppa-dev-tools` 工具集了。请确保您的系统已经启用了 `universe` 软件包源，因为 `ppa-dev-tools` 可能依赖于其中的一些软件包。

创建ppa

```sh
ppa create my-ppa
```



## 准备源包

### 准备源代码

首先，我们创建一个包含基本源文件的目录：

```sh
$ mkdir testpackage
$ cd testpackage
```

### 生成文件

对于那些不熟悉它的人来说，makefile 是一个定义与源代码相关的操作的文件。

在这种情况下，我们（基本上）关注两个操作：

- `all`：编译源代码;
- `install`：安装它。

以下命令将创建一个包含这两个操作的基本 makefile：

```sh
$ echo -e 'BINDIR := /usr/bin

all:
\tgcc main.c -o my_hello_world

install:
\tmkdir -p ${DESTDIR}${BINDIR}
\tcp my_hello_world ${DESTDIR}${BINDIR}/
' > Makefile
```

几点说明：

1. 引用是必需的，因为构建系统有自己的工作目录（用于编译、安装等），因此 makefile 必须提供自定义它的选项;当用户通过 Make 在其环境上构建/编译时，未指定，因此不会影响其操作;`$DESTDIR``$DESTDIR`
2. 操作（操作）通过 Tab 缩进;使用空格会导致错误;
3. 如果我们要复制到 ，则需要额外的操作，因此我们选择作为示例。`/usr/local/bin``/usr/bin`

需要注意的重要一点是，构建系统将通过检查 的结果自动推断已安装的文件结构;因此，对于简单的包，开发人员不需要显式指定任何内容。`make install`

#### Debian 打包元数据创建和核心概念

现在我们需要创建包元数据模板：

```sh
$ dh_make -p testpackage_0.0.0.1 --single --native --copyright mit --email xxx@xxx.com
$ rm debian/*.ex debian/*.EX 		# these files are not needed
```

这将创建具有指定值的 Debian 软件包元数据模板;将所有参数替换为预期的参数。

元数据以多个文件的形式包含在（新创建的）目录中，其中包含有关包和所涉及人员的信息，以及构建/安装的（编程）说明。`debian/`

绝对必要的文件是：

- `changelog`：更新日志条目列表，以及一些其他元数据，包括分布
- `control`：包元数据的主体：依赖项、描述、链接......
- `copyright`

##### 更新`changelog`

对于第一个版本，第一个条目是预填充的，因此我们只需要更改：

```sh
$ perl -i -pe "s/unstable/$(lsb_release -cs)/" debian/changelog
```

更改的参数称为“分布”，在术语的“通道”意义上（参见 [Debian 参考）;](https://www.debian.org/doc/manuals/developers-reference/pkgs.html#distribution)我们用当前的开发人员 O/S 发行版替换它。

后续的更新日志可以通过该工具执行。`dch`

##### 更新`control`

这里有很多变化要做。

一[、该部分](https://www.debian.org/doc/manuals/developers-reference/resources.html#archive-sections)：

```sh
$ perl -i -pe 's/^(Section:).*/$1 utils/' debian/control
```

我们只使用通用部分。`utils`

然后，Web 引用：

```sh
$ perl -i -pe 's/^(Homepage:).*/$1 https:\/\/testpackage.barryfoo.org/'              debian/control
$ perl -i -pe 's/^#(Vcs-Browser:).*/$1 https:\/\/github.com\/barryfoo\/testpackage/' debian/control
$ perl -i -pe 's/^#(Vcs-Git:).*/$1 https:\/\/github.com\/barryfoo\/testpackage.git/' debian/control
```

请注意，对于 ，Debian 的约定是首选源代码而不是 ，因为第一个（被认为）必然是公开的。`Vcs-Git``https``git`

现在的描述：

```sh
$ perl -i -pe 's/^(Description:).*/$1 A short description/'                               debian/control
$ perl -i -pe $'s/^ <insert long description.*/ A long description,\n very long indeed!/' debian/control
```

长描述的每一行都必须缩进一个空格。

最后，家政服务：

```sh
$ perl -i -pe 's/^(Standards-Version:) 3.9.6/$1 3.9.7/' debian/control
```

因为，至少在 Xenial 上，使用的默认版本 （3.9.6） 不是最新版本，并且会在构建软件包时发出警告。

##### 更新`copyright`

只需输入基本信息（当前年份、作者姓名和电子邮件）：

```sh
$ perl -i -0777 -pe "s/(Copyright: ).+\n +.+/\${1}$(date +%Y) Barry Foo <xxx@xxx.com>/" debian/copyright
```

### 生成源包

我们现在准备好了！让我们构建包：

```
$ debuild -S | tee /tmp/debuild.log 2>&1  	# log file used in the next section
```

这将在当前目录的父目录中创建几个文件：

```
testpackage_0.0.0.1.dsc
testpackage_0.0.0.1_source.build
testpackage_0.0.0.1_source.changes
testpackage_0.0.0.1.tar.xz
```

和 文件分别包含包元数据和更改日志;两者都已签名。`dsc``changes`

## 上传包

现在，软件包 [files] 可以上传到 PPA，进行构建，然后发布：

```sh
$ dput ppa:rookie0422/my-ppa <source.changes> 
```

如果一切正常，将向 PGP 电子邮件发送一封电子邮件（通常在几分钟内），通知包 [版本] 已被接受。

设置与 PGP 安全性相关的所有内容**至关重要**（请参阅上面的特定部分），否则，Launchpad 可能会意外采取行动 - 在最坏的情况下，接受包但不会在没有任何警告的情况下发布它，并阻止对上传的版本进行任何进一步的操作。

在后台，将生成包，并发送另一封电子邮件，其中包含有关生成最终状态（成功/失败）的通知。

默认情况下，构建是针对 amd64 和 i386 架构执行的;日志可以在包详细信息页面中找到：https://launchpad.net/~barryfoo/+archive/ubuntu/testppa/+packages。

## 删除包

如果操作拙劣，可以通过 Launchpad 界面删除包 [版本]：

- 转到 PPA 页面
- 点击（右上）：https://launchpad.net/~barryfoo/+archive/ubuntu/testpackage/+packages`View Package details`
- 点击（右上角）：https://launchpad.net/~barryfoo/+archive/ubuntu/testpackage/+delete-packages`Delete packages`

## 使用 PPA

PPA 和第一个软件包版本现已准备就绪！

可以像往常一样执行安装：

```
$ sudo add-apt-repository ppa:barryfoo/testppa
$ sudo apt update
$ sudo apt install testpackage
```







# fedora

[用户文档 — COPR 文档 (pagure.org)](https://docs.pagure.org/copr.copr/user_documentation.html#)







要将自己的软件添加到 Fedora 的软件源中，你可以使用 COPR（Cool Other Package Repositories）来实现。以下是简要的步骤：

## **准备软件包：**



以下是将 shell 脚本文件打包成 RPM 软件包的基本步骤：

1. **准备 RPM 构建环境**： 在你的系统上安装 RPM 构建所需的工具和依赖项。在 Fedora 上，你可以使用以下命令安装：

   ```
   sudo dnf install rpm-build rpmdevtools
   ```

2. **创建 RPM 构建目录结构**： 使用 `rpmdev-setuptree` 命令来创建 RPM 构建目录结构。在终端中执行以下命令：

   ```
   rpmdev-setuptree
   ```

   这将在你的家目录下创建一个名为 `rpmbuild/` 的目录，其中包含了 RPM 构建所需的子目录。

3. **将 shell 脚本文件复制到构建目录**： 将你的 shell 脚本文件复制到 RPM 构建目录的 `SOURCES/` 子目录中。在终端中执行以下命令：

   ```
   cp my_package/bin/test.sh ~/rpmbuild/SOURCES/
   ```

   这将把 `test.sh` 文件复制到 `~/rpmbuild/SOURCES/` 目录中。

   总之需要将软件例如`.c`,`.sh`,`.py`等文件放入`~/rpmbuild/SOURCES/` 目录中。

4. **创建 RPM Spec 文件**： 创建一个 RPM Spec 文件，它描述了 RPM 软件包的元数据和构建规则。可以使用文本编辑器创建一个名为 `my_package.spec` 的文件，并根据需要添加以下内容：

   ```sh
   Summary: My Package Summary
   Name: my_package
   Version: 1.0
   Release: 1%{?dist}
   License: Your License
   URL: https://example.com/my_package
   Source0: test.sh	#如果是单个文件直接指定软件名即可
   
   %description
   My Package Description
   
   %prep
   
   # 如果是不需要构建的比如.sh文件则不需要执行任何操作
   #如果是需要构建的比如.c文件，则需要使用cp命令将hello.c复制到%_builddir目录
   #cp %{SOURCE0} .
   
   
   %build
   
   # 如果是不需要构建的比如.sh文件则不需要执行任何操作
   #如果是需要构建的比如.c文件，则需要使用相应的操作
   #gcc -o hello hello.c
   
   %install
   # 将脚本复制到需要安装到的路径
   mkdir -p %{buildroot}/usr/bin
   cp %{SOURCE0} %{buildroot}/usr/bin/test.sh
   
   #or
   #mkdir -p %{buildroot}%{_bindir}
   #install -m 755 hello %{buildroot}%{_bindir}
   
   %files
   %doc
   /usr/bin/test.sh
   # 或者 %{_bindir}/hello-wm
   
   %changelog
   * Mon Apr 8 2024 frj <1712963739@qq.com> - 2.0-1
   - Initial package release
   
   ```

   请根据你的软件包的实际情况进行修改和自定义。保存文件并将其放置在 `~/rpmbuild/SPECS/` 目录中。

5. **构建 RPM 软件包**： 在终端中，进入到 RPM 构建目录，并使用 `rpmbuild` 命令来构建 RPM 软件包。执行以下命令：

   ```
   rpmbuild -bs ~/rpmbuild/SPECS/test.spec
   ```

   这将根据你的 Spec 文件执行构建操作，并在构建完成后生成 RPM 软件包。构建过程可能需要一些时间。



## **创建 COPR 账户：** 

在 https://copr.fedorainfracloud.org/ 上注册一个账户。这个网站是 Fedora 社区维护的 COPR 服务。

## **安装 COPR CLI 工具：** 

在你的 Fedora 系统上安装 COPR 命令行工具，它可以帮助你上传软件包到 COPR 仓库。使用以下命令安装：

```
sudo dnf install -y copr-cli
```

## **登录并创建仓库：**

 

1. 在此处设置 FAS 帐户：[https://accounts.fedoraproject.org](https://accounts.fedoraproject.org/)。

2. 登录 COPR（COPR 主页右上角的链接：https://copr.fedorainfracloud.org/）。

3. 访问 https://copr.fedorainfracloud.org/api/。获得api令牌

4. 将生成的身份验证令牌复制到文件中。`~/.config/copr`

   ```sh
   vi ~/.config/copr 
   ```

   将步骤3中获得的令牌加入copr文件中

5. 安装 copr-cli 工具：（如果您使用的是 Fedora）。

   ```sh
   sudo dnf install copr-cli
   ```

   

6. 运行以创建您的第一个项目。

   ```sh
   copr-cli create first-project --chroot fedora-rawhide-x86_64
   copr-cli create my-test-project --chroot fedora-39-x86_64  #根据系统版本
   ```

7. 在设置中记得勾选与系统版本相对应的Chroots

   Home --rookie0422  --my-test-project -- Settings



## **上传软件包：** 

一旦创建了仓库，你可以使用 `copr-cli` 工具上传你的软件包。在软件包的目录下，执行以下命令：

```sh
copr build first-project ~/rpmbuild/SRPMS/test-1.0-1.fc39.src.rpm
```

这将把你的软件包上传到你创建的 COPR 仓库中。

## **等待构建完成：** 

构建过程可能需要一些时间。一旦构建完成，你的软件包就可以在 COPR 仓库中找到了。



## **分享你的仓库：** 

安装仓库

```sh
sudo dnf copr enable username/repository_name
#eg
sudo dnf copr enable rookie0422/first-project
```



更新软件源

```sh
sudo dnf update
```



下载软件

```sh
sudo dnf install package_name
```





