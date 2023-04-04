//detect vertical scrollbars
var gResult = '';  

function isScrollBar(elem) {
  const style = getComputedStyle(elem);
  if (style.overflowY === 'scroll') return true;
  if (style.overflowY === 'auto') return true;
  if (style.overflow === 'scroll') return true;
  if (style.overflow === 'auto') return true;
  return false;
}

function hasVertScrollBar(elem) 
{
  if (!(elem instanceof Element)) 
    return false;
  
  if (typeof elem.scrollTop === undefined)
    return false;
  
  if (typeof elem.scrollWidth === undefined) 
    return false;
     
  if (elem.scrollTop > 0)
  {
    return true;
  }

  if (elem.scrollWidth > elem.clientWidth)
  {
    if (isScrollBar(elem))
      return true;
  }

  return false;
} 

function checkLevel(elem) {
  
  if (typeof elem === undefined) 
    return;

  if (elem.hasChildNodes)
  { 
    var i=0; 
    var nds = elem.childNodes;
    for (i=0; i < nds.length; i++)  
    {
      if (typeof nds[i] === undefined) 
      {
        continue;
      } 
      
      if (hasVertScrollBar(nds[i])) 
      {
        var scrollElement = nds[i];
        var rect = scrollElement.getBoundingClientRect();
        var win = scrollElement.ownerDocument.defaultView;
        var left = rect.left + win.pageXOffset;
        var scrollbarpos = (left + rect.width); 
        gResult += Math.round(scrollbarpos);
        gResult += ';';
      }
      
      if (nds[i].hasChildNodes)
      { 
        checkLevel(nds[i]);
      }
    }
  }
}

function findScrollBar()
{
  checkLevel(document.documentElement); 
  return gResult; 
}

findScrollBar();
