// ILA in line attachment changes
function ILAexpandThumb(thumbID) {
	// basically the same as expandThumb, but maintains the style width and height attributes instead of feeding them to the bears
	var img = document.getElementById('thumb_' + thumbID);
	var link = document.getElementById('link_' + thumbID);
	
	// save the currently displayed image attributes
	var tmp_src = img.src;
	var tmp_height = img.style.height;
	var tmp_width = img.style.width;
	
	// set the displayed image attributes to the link attributes, this will expand in place
	img.src = link.href;
	img.style.width = link.style.width;
	img.style.height = link.style.height;
	
	// save the image attributes back into the link pop back when clicked again
	link.href = tmp_src;
	link.style.width = tmp_width;
	link.style.height = tmp_height;
	
	return false;
}