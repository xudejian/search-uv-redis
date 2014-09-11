@xdf = {}

loadAd = (data, dom_id) ->
  if data
    return if data.empty
    return if data.error
    render(data, dom_id)

dataToObj = (data) ->
  click_url: data[0]
  show_url: data[1]
  img: data[2]
  img_desc: data[3]
  txt_title: data[4]
  txt_desc1: data[5]
  txt_desc2: data[6]
  id: data[7]
  type: data[8]
  width: data[9]
  height: data[10]

get_domain = (url) ->
  offset = url.indexOf('://')
  if -1 isnt offset
    url = url.substr offset+3
  offset = url.indexOf('/')
  if -1 isnt offset
    url = url.substr 0, offset
  url

render_baidu_wenzi = (data) ->
  html = ['<ul style="list-style:none;margin:0">']
  a_style = 'display:block;font-size:1em; overflow:hidden; white-space:nowrap'
  for i in data.set when i
    ad = dataToObj i
    html.push [
      '<li style="width:270px;font-size:medium;line-height:1.15em;margin-bottom:15px">'
      '<a target="_blank" style="'
      a_style
      '" href="'
      ad.click_url
      '">'
      ad.txt_title
      '</a>'
      '<a target="_blank" style="'
      'font-size:small'
      '" href="'
      ad.click_url
      '">'
      '<font size="-1" style="color:#000">'
      ad.txt_desc1
      '</font>'
      '<br>'
      '<font size="-1" style="color:#080">'
      (ad.show_url || get_domain(ad.click_url))
      '</font>'
      '</a>'
      '</li>'
    ].join('')
  html.push '</ul>'
  html.join('')

render_wenzi = (data) ->
  html = []
  for i in data.set when i
    ad = dataToObj i
    html.push [
      '<a target="_blank" style="color:#666;font-family:宋体;font-size:12px;margin-left:20px" href="'
      ad.click_url
      '">'
      ad.txt_title
      '</a>'
    ].join('')
  html.join('')

render_hengfu = (data) ->
  html = []
  for i in data.set when i
    ad = dataToObj i
    html.push [
      '<a style="border:0" target="_blank" href="'
      ad.click_url
      '"><img src="'
      ad.img
      '" alt="'
      ad.img_desc
      '"></a>'
    ].join('')
  html.join('')

render_mn_pic = (data) ->
  html = ['<ul style="list-style:none;margin:0">']
  a_style = 'display:block; text-align:center; font-size:12px; line-height:26px; color:#003a7d; width:120px; overflow:hidden; white-space:nowrap'
  for i in data.set when i
    ad = dataToObj i
    html.push [
      '<li style="float:left;display=inline;width:120px;padding:15px 0 0 30px">'
      '<a target="_blank" style="'
      a_style
      '" href="'
      ad.click_url
      '"><img style="width:120px; height:90px" src="'
      ad.img
      '" alt="'
      ad.img_desc
      '"></a>'
      '<a target="_blank" style="'
      a_style
      '" href="'
      ad.click_url
      '">'
      ad.img_desc
      '</a>'
      '</li>'
    ].join('')
  html.push '</ul>'
  html.join('')

render_carousel = (data) ->
  html = []
  for i in data.set when i
    ad = dataToObj i
    html.push [
      '<a style="border:0" target="_blank" href="'
      ad.click_url
      '"><img src="'
      ad.img
      '" alt="'
      ad.img_desc
      '"></a>'
    ].join('')
  html.join('')

render_pair = (data) ->
  html = []
  for i in data.set when i
    ad = dataToObj i
    html.push [
      '<a target="_blank" href="'
      ad.click_url
      '"><img src="'
      ad.img
      '" alt="'
      ad.img_desc
      '"></a>'
    ].join('')
  html.join('')

render_float = (data) ->
  html = [
    '<div style="position:fixed;right:10px;bottom:10px;width:'
    data.width
    'px;'
    'height:'
    data.height
    'px'
    '">'
  ]
  for i in data.set when i
    ad = dataToObj i
    html.push [
      '<a target="_blank" href="'
      ad.click_url
      '"><img src="'
      ad.img
      '" title="'
      ad.img_desc
      '"></a>'
    ].join('')
  html.push '</div>'
  html.join('')

empty = -> ''

render_fn =
  '7': render_wenzi
  '2': render_hengfu
  '3': render_mn_pic
  '4': render_carousel
  '5': render_pair
  '6': render_float
  '1': render_baidu_wenzi

render = (data, dom_id) ->
  slot = $("#"+dom_id)
  slot.html (render_fn[data.tpl]||empty) data

  css = {}
  if data.tpl isnt '6'
    if data.width
      css.width = data.width + 'px'
    if data.height
      css.height = data.height + 'px'
    css.border = 'solid 1px green'
  if data.AdLeft
    css.left = data.AdLeft
  if data.AdTop
    css.top = data.AdTop
  if data.AdRight
    css.right = data.AdRight
  if data.AdBottom
    css.bottom = data.AdBottom
  slot.css css

@xdf.fillSlot = (id) ->
  slot_id = 'xdf_slot_' + id
  document.write("<div id='"+slot_id+"'></div>")
  cb = (data) -> loadAd data, slot_id
  $.getJSON "http://ads.staff.xdf.cn/demo/#{id}?callback=?", cb
