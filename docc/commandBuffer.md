![1666842331819](image/commandBuffer/1666842331819.png)

commandBuffer在begin和end之间录制cmd命令

录制好的命令会在 `vkQueueSubmit`提交到GPU执行

每一帧都会提交到gpu重复执行
