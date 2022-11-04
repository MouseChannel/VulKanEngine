### `cmdBeginRenderPass`

该命令会开启渲染流程

主要传入两个参数

- `renderPass`
- `swapchain `里面的 `frameBuffer`







### 什么是renderpass

renderPass代表了一整套的渲染流程

renderpass类似一个管理器，里面存了所有的配置 :`AttachmentDescription`

这些配置表明了对深度图，采样之前的图，和采样之后最终要显示的图，的一些操作，包括：

- 读取完和储存完之后的操作 `loadOp, storeOp`
- 该图片刚刚加载时的layout和使用时要转化成的layout
- 等等

subpass类似实例，会挂载renderpass里的某些现有的配置


### 什么是frameBuffer

frameBuffer包含了一帧需要被渲染的所有图的 `imageView`

- 深度图，采样之前的图，和采样之后最终要显示的图


---



在commandBuffer执行命令时，首先先从frameBuffer里面拿到这些imageView，然后再从subpass上的 `AttachmentDescription`上拿该图片的配置信息，因为是分开设置保存在不同的数组里。因此

`frameBuffer里面的imageView数组 和 renderPass里的AttachMentDescription数组的下标需要一一对应`
