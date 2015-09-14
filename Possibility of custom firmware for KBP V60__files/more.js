$(document).ready(function()
{
  $('.more').hide();
  $('.more_head').click(function()
  {
    if ($(this).next('.more_body').css('display') == "none")
    {
      $(this).parent('.signature').css('max-height', 'none');
    }
    $(this).next('.more_body').slideToggle(600, function()
    {
      if ($(this).css('display') != "none")
      {
        $(this).prev('.more_head').text("Less");
      }
      else
      {
        $(this).prev('.more_head').text("More");
        $(this).parent('.signature').css('max-height', '120px');
      }
    });
  });
  $('.more_image').click(function()
  {
    $(this).next('.more_body').slideToggle(600, function()
    {
      if ($(this).css('display') != "none")
      {
        $(this).prev('.more_image').text("Hide Image");
      }
      else
      {
        $(this).prev('.more_image').text("Show Image");
      }
    });
  });
});
