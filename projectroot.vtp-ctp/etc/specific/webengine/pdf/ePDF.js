var qt = {
	replacePDFLinks: function ()
		{
			var listIframes = document.getElementsByTagName("iframe");
			for (var i = 0; i < listIframes.length; i++) {
				if ((listIframes[i].src.slice(-4)=='.pdf') && (listIframes[i].src.indexOf("?")==-1))
				{
					//console.log(listIframes[i].src);
				   listIframes[i].src = 'file:////etc/specific/webengine/pdf/web/viewer.html?file=' + listIframes[i].src;
				}
			}
				
			var listHRefs = document.getElementsByTagName("a");
			for (var i = 0; i < listHRefs.length; i++) {
				if ((listHRefs[i].href.slice(-4)=='.pdf') && (listHRefs[i].href.indexOf("?")==-1))
				   listHRefs[i].href = 'file:////etc/specific/webengine/pdf/web/viewer.html?file=' + listHRefs[i].href;
			}
		}
};
document
