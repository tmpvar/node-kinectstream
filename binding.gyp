{
  'targets': [
    {
      'target_name': 'kinect',
      'sources': [
        'src/binding.cc', 'src/kinect.cc'
      ],
      'libraries': [
         'libfreenect.a',
      ],
    }
  ]
}