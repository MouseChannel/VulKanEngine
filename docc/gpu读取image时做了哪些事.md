vkImage

1. 创建句柄 `VkImage`
2. gpu分配显存空间 `VkDevicememory`，同时将句柄与该显存空间绑定
3. 通过句柄 `VkImage`生成 `VkImageView`
4. 创建一个 临时 `buffer`，并将图片内容从cpu端map到该 `buffer`，然后再将该buffer提供给该 `vkImage`，作为该图片的内容

---

#### layout的理解

代表图片的存储格式，当图片被用于不同的用途时，会被硬件做相对应的无损压缩，因此对图片操作，必须将其手动转化到对应的格式

- `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ` 适合往该图片上渲染像素，一般最后要渲染的图片会转换为这种格式
- `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`  作为对象，一般在已经申请完空间，往该图片显存里传输数据时
- `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`  输入到shader里面的
