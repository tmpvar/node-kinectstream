{
  'targets': [
    {
      'target_name': 'kinect',
      'sources': [
        'src/binding.cc',
      ],
      'libraries': [
         'libfreenect.a',
      ],
    }
  ]
}