 每一帧的渲染流程

* vkAcquireNextImageKHR —— 从 SwapChains 获取下一个可以绘制到屏幕的Image
* vkResetCommandPool/vkResetCommandBuffer —— 清除上一次录制的 CommandBuffer，可以不清但一般每帧的内容都可能发生变化一般都是要清理的。
* vkBeginCommandBuffer —— 开始录制
* vkCmdBeginRenderPass —— 启用一个RenderPass，这里就连同绑定了一个 FrameBuffer
* vkCmdBindPipeline —— 绑定Pipeline，想要换shader就得在这里做
* vkCmdBindDescriptorSets —— 绑定 DescriptorSets，可以一次绑多个Set，也可以多次绑定多个Set，同时需要给出 PipeLineLayout
* vkCmdBindVertexBuffers&vkCmdBindIndexBuffer —— 没啥好多说的了，绑模型
* vkCmdDrawIndexed —— 最关键的绘制命令，这里可以根据显卡的特性支持情况换更高级的绘制命令比如[indirect](https://www.zhihu.com/search?q=indirect&search_source=Entity&hybrid_search_source=Entity&hybrid_search_extra=%7B%22sourceType%22%3A%22answer%22%2C%22sourceId%22%3A1632072443%7D)，相应的数据绑定也需要改。
* vkCmdEndRenderPass —— 结束 RenderPass
* vkEndCommandBuffer —— 结束 CommandBuffer
* vkQueueSubmit —— 提交执行渲染任务
* vkQueuePresentKHR —— 呈现渲染数据，这时候调用可能 vkQueueSubmit 还没执行完，但 Semaphores 会帮我们打点好。
