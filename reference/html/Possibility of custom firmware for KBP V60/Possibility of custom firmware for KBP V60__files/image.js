/*
 * Scale an image to a rectangle while maintaining the aspect ratio.
 */
function img_downscale(img, width, height)
{
	factor = (img.width > img.height) ? width / img.width : height / img.height;
	if (factor < 1)
		img.width = img.width * factor;
};
