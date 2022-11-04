#### 区分管线与实际的绘制命令

当 `commandBuffer`执行一个transfer命令时，执行的设想该命令的经过的stage为

- top_Pipeline
- Transfer
- bottom_pipeline

实际上不是的。

**实际上只要是个命令，它就会流过管线的所有步骤**

如transfer命令，也会流过renderPass的所有stage，只不过该 `commandBuffer`没有操作要执行而已

同样 `begin_Render_Pass`命令也会经过transfer阶段

#### barrier同步机制

commandBuffer和commandBuffer之间的同步通过 `VkSemaphore`实现，如**渲染**命令需要等**显示**完后，该frameBuffer空闲，才可以往上面渲染

commandBuffer自己的同步通过 `vkFence`实现，如某个命令在执行时，需要等执行才可以再次被提交

pipeline各步骤之间的同步需要通过memoryBarrier实现。如某个步骤需要将数据拷贝到 `VkImage`对应的显存里，则必须要等到该image的layout转化成 `Transfer_dst`格式

使用barrier需要用到的参数

- `srcStage`流水线的某个阶段
- `dstStage`
- `srcAccess`某个操作，如write，read
- `dstAccess`

该commandbuffer需要在 `srcStage`的 `srcAccess`操作之后执行，同时在 `dstStage`的 `dstAccess`的操作前执行完。

即commandBuffer会先阻塞，被 `srcAccess`唤醒

同时 `dstAccess`会阻塞，被commandBuffer唤醒

##### 以深度图举例

commandBuffer = 将图片的layout改为Depth的格式

 对深度图的第一次操作是在 `Fragment_Shader`之前的 `early_Test`，往上面写深度信息。因此在这之前执行完commandBuffer

即 `dstStage = early_test`。`dstAccess = write`

src则没有要求，因此将 `srcstage`设置为第一步 `top_pipeline`，然后 `srcAcccess = 0`，即等待 `空`操作完成后，即 `top_pipeline`的第一时间执行
