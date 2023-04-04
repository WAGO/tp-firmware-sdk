//detect horizontal scrollbars
var gResult = '';  

function isScrollBar(elem) {
  const style = getComputedStyle(elem);
  if (style.overflowX === 'scroll') return true;
  if (style.overflowX === 'auto') return true;
  if (style.overflow === 'scroll') return true;
  if (style.overflow === 'auto') return true;
  return false;
}

function hasHorizScrollBar(elem) 
{
  if (!(elem instanceof Element)) 
    return false;
  
  if (typeof elem.scrollLeft === undefined)
    return false;
  
  if (typeof elem.scrollHeight === undefined) 
    return false;
     
  if (elem.scrollLeft > 0)
  {
    return true;
  }

  if (elem.scrollHeight > elem.clientHeight)
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
      
      if (hasHorizScrollBar(nds[i])) 
      {
        var scrollElement = nds[i];
        var rect = scrollElement.getBoundingClientRect();
        var win = scrollElement.ownerDocument.defaultView;
        var bottom = rect.top + win.pageYOffset;
        var scrollbarpos = (bottom + rect.height); 
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
