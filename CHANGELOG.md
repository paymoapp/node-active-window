# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

### [0.1.1-8](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-7...v0.1.1-8) (2022-06-28)


### Build/CI

* **gitlab:** Publish to gitlab registry ([ee551bc](https://gitlab.paymoapp.com/gergo/node-active-window/commit/ee551bc33b5d18a29be7ae53443d5901edc15bda))

### [0.1.1-7](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-6...v0.1.1-7) (2022-06-24)

### [0.1.1-6](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-5...v0.1.1-6) (2022-06-24)


### Build/CI

* Using img-ubuntu-widget-build for building the linux addon ([e0857f8](https://gitlab.paymoapp.com/gergo/node-active-window/commit/e0857f8617217d646ee0fc614a63f4e8e7aee9f0))

### [0.1.1-5](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-4...v0.1.1-5) (2022-06-23)

### [0.1.1-4](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-3...v0.1.1-4) (2022-06-23)


### Build/CI

* **gitlab:** Use --target_arch at publish time ([d3d2ec2](https://gitlab.paymoapp.com/gergo/node-active-window/commit/d3d2ec2a71722b2ae5ad0291f2a2fb2af1105f05))

### [0.1.1-3](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-2...v0.1.1-3) (2022-06-23)


### Build/CI

* **gitlab:** M1 has arm64 arch not arm ([67475d7](https://gitlab.paymoapp.com/gergo/node-active-window/commit/67475d7ff7f2d48bd0aca18baca38d0e071dae54))

### [0.1.1-2](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-1...v0.1.1-2) (2022-06-23)


### Build/CI

* Fixing mac build ([00dc5ca](https://gitlab.paymoapp.com/gergo/node-active-window/commit/00dc5cac83ecafb89c2012836de699b27dfd3a2c))

### [0.1.1-1](https://gitlab.paymoapp.com/gergo/node-active-window/compare/v0.1.1-0...v0.1.1-1) (2022-06-23)

### 0.1.1-0 (2022-06-22)


### Features

* Added javascript-side demo ([7503dce](https://gitlab.paymoapp.com/gergo/node-active-window/commit/7503dce9ed655d8d0535c0a834fc161e874fdef4))
* Enabled prebuilt binding files using node-pre-gyp ([a8de768](https://gitlab.paymoapp.com/gergo/node-active-window/commit/a8de768868ef9e34a3a75f983c1f7ea9f92b5c43))
* **module.linux:** Added bindings for subscribe and unsubscribe, modified demo and javascript library to support subscriptions ([d26fbc9](https://gitlab.paymoapp.com/gergo/node-active-window/commit/d26fbc96a00d7c911ea644a4d5bb2eb922e18f71))
* **module.linux:** Added napi mappings for linux library, added initialize function ([9018519](https://gitlab.paymoapp.com/gergo/node-active-window/commit/901851901a32269cb8586886ccb39f138051a823))
* **module.linux:** Added watch mode with background thread and callbacks ([56f4276](https://gitlab.paymoapp.com/gergo/node-active-window/commit/56f42765db4a25b6fa80db7645668d8f4026c6ee))
* **module.linux:** Get active window and active window title using xlib ([2c76920](https://gitlab.paymoapp.com/gergo/node-active-window/commit/2c7692040ac59ca60504a8f75fd2db8b88aa290c))
* **module.linux:** Get application icon using app cache built during startup ([26bc9fe](https://gitlab.paymoapp.com/gergo/node-active-window/commit/26bc9feb4ed37ad7d888a6be0eb7adcae9a6aa40))
* **module.linux:** Get title, pid and application name - in this case, class - of active window ([3c86011](https://gitlab.paymoapp.com/gergo/node-active-window/commit/3c860119e2e246b038fd92764c769ebfc5d745d7))
* **module.linux:** Loading all application desktop entries and resolving the icon paths for them ([a60feff](https://gitlab.paymoapp.com/gergo/node-active-window/commit/a60feffec25ceb7a4789db5339cedfc7773258e3))
* **module.macos:** Added napi mappings for macos library ([82b18fe](https://gitlab.paymoapp.com/gergo/node-active-window/commit/82b18fe0114a4b01fafc0729f189489f674c61c6))
* **module.macos:** Added watch mode with hacky runloop ([1fcba28](https://gitlab.paymoapp.com/gergo/node-active-window/commit/1fcba2896f3022bda6f7a07536f6c7f8b5788d53))
* **module.macos:** Implemented bindings and added javascript code that continuously calls the NSRunLoop when it detects darwin ([9d49792](https://gitlab.paymoapp.com/gergo/node-active-window/commit/9d49792f0abbe86904f327b78dbdbb7cb6821881))
* **module.macos:** Implemented macos library to get active window info ([635043b](https://gitlab.paymoapp.com/gergo/node-active-window/commit/635043b1fdd48e4b501ab034ee6793cc10e22bbd))
* **module.windows:** Added bindings for subscription ([658c277](https://gitlab.paymoapp.com/gergo/node-active-window/commit/658c277cf3fef493f9e52633f06a1a55d2647c32))
* **module.windows:** Added napi binding ([4796d5f](https://gitlab.paymoapp.com/gergo/node-active-window/commit/4796d5f4e6e107f51ad59f2f7a13a6d344b625b3))
* **module.windows:** Added watch mode with background thread ([8fceff0](https://gitlab.paymoapp.com/gergo/node-active-window/commit/8fceff0566c800d34f5fd3e6149452fbcfa157cc))
* **module.windows:** Extract application icon from exe and convert it to base64 encoded png data url ([30f8715](https://gitlab.paymoapp.com/gergo/node-active-window/commit/30f8715f5cd7606aa68071e871d4153a875cebe8))
* **module.windows:** Extract application icon from executable and save it in a file for now *WIP* ([2c14d1b](https://gitlab.paymoapp.com/gergo/node-active-window/commit/2c14d1bb7d909d40cdaaa93b0ed21cf30c2da6b2))
* **module.windows:** Extract application icon from UWP apps ([febb0e3](https://gitlab.paymoapp.com/gergo/node-active-window/commit/febb0e34bfecea1b79d6eca936a72f2a402838cf))
* **module.windows:** Get application name and path for UWP apps ([395fae3](https://gitlab.paymoapp.com/gergo/node-active-window/commit/395fae325b535aed4b4d6b6d7a6657bd1281b4ff))
* **module.windows:** Get window in foreground and it's title, application name, application path and PID ([5e84d0c](https://gitlab.paymoapp.com/gergo/node-active-window/commit/5e84d0c152f5981481370cfab56057dce8838156))
* Updated javascript code to support requesting permissions ([634759a](https://gitlab.paymoapp.com/gergo/node-active-window/commit/634759a1c56968311188bea34b7d6e6b2462b041))


### Bug Fixes

* **demo:** Added request permissions call ([2bca603](https://gitlab.paymoapp.com/gergo/node-active-window/commit/2bca6037abc2a88a245db33a555d8157c47063bf))
* **library:** The callback for addon.subscribe can get null and the value needs to be encoded ([1d2d9b6](https://gitlab.paymoapp.com/gergo/node-active-window/commit/1d2d9b6831b2fbf73e2111ce74d8bda6ba19d352))
* **module.macos:** Check if we have macOS catalina to check screen capture permissions ([3a36423](https://gitlab.paymoapp.com/gergo/node-active-window/commit/3a36423140a3b8cefd4d4c5583def1ba8c477b9f))
* **module.macos:** Implemented RunLoop to update frontmost window ([e2a93f7](https://gitlab.paymoapp.com/gergo/node-active-window/commit/e2a93f77fda78a0ac9bf3494dc99a6a02392160d))
* **module.windows:** Also watch for title changed events ([c3864a1](https://gitlab.paymoapp.com/gergo/node-active-window/commit/c3864a1f34d4bd1fa522c67099cda1c80a8dbb0e))
* **module.windows:** Clean up after itself ([f3c8618](https://gitlab.paymoapp.com/gergo/node-active-window/commit/f3c86188a79bf9d5b688e0f179be93e55737f47c))
* **module.windows:** Close process handle ([31881ca](https://gitlab.paymoapp.com/gergo/node-active-window/commit/31881cac636a8a791c0523fa40ce4f13f7212f6a))
* **module.windows:** Don't crash when UWP app is not yet initialized ([c0911e8](https://gitlab.paymoapp.com/gergo/node-active-window/commit/c0911e8ccb788ddcf657304d0bd898e2d3ecb60b))
* **module.windows:** Fixed buffer overflow in getWindowTitle and replaced &buf[0] to buf.data() to access raw array of vectors ([bd70a97](https://gitlab.paymoapp.com/gergo/node-active-window/commit/bd70a9722c8c508419f53e9331f63cfaf7294dce))


### Documentation

* Added documentation for JS library ([8dd91dc](https://gitlab.paymoapp.com/gergo/node-active-window/commit/8dd91dc19370ebe1a5b3fd39a9df3a94387a217a))
* Added documentation for windows and macos native modules ([5fd30ce](https://gitlab.paymoapp.com/gergo/node-active-window/commit/5fd30ce6bd9c20f7b036ac2c834f82dcf6c5ede5))
* **module.linux:** Added documentation for linux native library ([e18a015](https://gitlab.paymoapp.com/gergo/node-active-window/commit/e18a015b8530043375ca6f8d0e71c6aa45f9a8ce))


### Build/CI

* **docs:** Added script to automatically generate README table of contents ([9d4878b](https://gitlab.paymoapp.com/gergo/node-active-window/commit/9d4878b2c7132f18c9893f3ec8a05b621c1d72e8))
* **gitlab:** Added gitlab-ci configuration ([4c50352](https://gitlab.paymoapp.com/gergo/node-active-window/commit/4c5035292971b9f8f7f495aad9ac627f8e4132f4))
* **gitlab:** Fix typo ([ff78131](https://gitlab.paymoapp.com/gergo/node-active-window/commit/ff78131ca80f9d011aa4f851617069c099367f05))
* Include N-API version in prebuilt binaries ([0b2ce15](https://gitlab.paymoapp.com/gergo/node-active-window/commit/0b2ce155e24d11aea0a5a18e3bf92b6d08c340ce))
* Initialized project ([d2f19a2](https://gitlab.paymoapp.com/gergo/node-active-window/commit/d2f19a25ae1be025762a82718e8157aafcf22c0c))
* **module.windows:** Use visual studio nmake to build demo instead of mingw ([e33fdb9](https://gitlab.paymoapp.com/gergo/node-active-window/commit/e33fdb900eef68abe9d116ff18e38d29998f574d))
