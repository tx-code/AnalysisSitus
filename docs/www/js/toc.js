function buildToc(container) {
  var html = "", lastLevel = -1;
  container.find('h1, h2, h3, h4, h5, h6, h7, h8, h9').each(function() {
    var $this = $(this);
    var level = this.tagName.match(/H(\d)/)[1];
    if (lastLevel < level) {
      html += "<ul class='light'>";
    }
    if (lastLevel > level)  {
      html += "</ul>";
    }
    html += "<li><a href='" + window.location.pathname + '#' + this.id + "' >" + $this.text() + "</a></li>";
    lastLevel = level;
  });
  return html;
}

// On DOM loaded.
document.addEventListener("DOMContentLoaded", () => {
  // Get ToC placeholder.
  toc = document.getElementById("toc");

  // Add a header.
  tocHeader = document.createElement("b");
  tocHeader.innerText = "Quick navigation";
  toc.appendChild(tocHeader);

  // Build ToC.
  $('#toc').append( buildToc( $('body') ) );

});



