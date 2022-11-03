GraphicQueue和PresentQueue里面存的都是录制好的commandBuffer

## 最简单的情况，假设GraphicQueue和presentQueue是同一个

1. 首先CPU需要通过 `vkAcquireNextImageKHR`获取可用的frameBuffer，即 `imageIndex`，也就是swapchain里面的 `m_SwapChainFrameBuffers`数组的索引
2. 通过 `vkQueueSubmit` 将用于渲染的 `commandBuffer`执行，传入的 `commandBuffer`里会绑定上面拿到 `imageIndex`，从而渲染到该frameBuffer上面，完成本帧的渲染工作
3. 通过 `vkQueuePresentKHR`，传入上一步拿到的frameBuffer，将该frameBuffer显示

### 加入同步机制

两个信号量

- `m_ImageAvailableSemaphore_1, m_ImageAvailableSemaphore_2`
- `m_RenderFinishedSemaphore_1, m_RenderFinishedSemaphore_2`

#### 单个frameBuffer的流程

1. `vkAcquireNextImageKHR`获取可用的frameBuffer，同时点亮 `m_ImageAvailableSemaphore_1`
2. `vkQueueSubmit`中的commandBuffer，等待 `m_ImageAvailableSemaphore_1`后执行， 且执行完后点亮 `m_RenderFinishedSemaphore_1`
3. `vkQueuePresentKHR`等待 `m_RenderFinishedSemaphore_1`执行，且执行完后点亮 `m_ImageAvailableSemaphore_1`，回到第一步，如果没执行完，则会一直阻塞在第一步，等待点亮 `m_ImageAvailableSemaphore_1`为止。

#### 多个frameBuffer的流程，以两个为例

1. `vkAcquireNextImageKHR`获取可用的frameBuffer，同时点亮 `m_ImageAvailableSemaphore_2`
2. `vkQueueSubmit`中的commandBuffer，等待 `m_ImageAvailableSemaphore_2`后执行， 且执行完后点亮 `m_RenderFinishedSemaphore_2`
3. `vkQueuePresentKHR`等待 `m_RenderFinishedSemaphore_2`执行，且执行完后点亮 `m_ImageAvailableSemaphore_2`，此时执行显示任务需要一定时间，程序不会阻塞等待该 `m_ImageAvailableSemaphore_2`，而是阻塞等待上一帧就已经执行的 `m_ImageAvailableSemaphore_1`，再重复第一步

---

#### commandBuffer的同步

此时frameBuffer用于渲染和显示的同步已经完成。但是

这里的commandBuffer是传引用，当被 `vkQueueSubmit`送到GPU执行渲染任务的时候，如果此时再次将该commandBuffer再次通过 `vkQueueSubmit`提交，会出错，执行中的commandBuffer无法再次被提交。因此需要对每个commandBuffer的提交做同步
